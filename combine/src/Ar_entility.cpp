#include <Ar_entility.h>

struct Voxel voxel;
struct Coord coord;
struct StartParams params;


const int IMG_WIDTH = 4032;
const int IMG_HEIGHT = 3024;
const int VOXEL_DIM = 128;
const int VOXEL_SIZE = VOXEL_DIM*VOXEL_DIM*VOXEL_DIM;
const int VOXEL_SLICE = VOXEL_DIM*VOXEL_DIM;
const int BACKGROUND = 0;


void Ar::generate_arucoboard(int markerX, int markerY, int marker_length, int marker_separation, int dictionary_id, int board_bits)
{
    m_markerX = markerX;
    m_markerY = markerY;
    m_marker_length = marker_length;
    m_marker_separation = marker_separation;
    m_dictionary_id = dictionary_id;
    m_board_bits = board_bits;

    cv::Size image_size;
    image_size.width = m_markerX * (m_marker_length + m_marker_separation) + m_marker_separation;
    image_size.height = m_markerY * (m_marker_length + m_marker_separation) + m_marker_separation;

    cv::Ptr<cv::aruco::Dictionary> dictionary = 
            cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(m_dictionary_id)); 
    m_dictionary = dictionary;
    cv::Ptr<cv::aruco::GridBoard> board = cv::aruco::GridBoard::create(m_markerX, m_markerY, float(m_marker_length),
                                                                    float(m_marker_separation), dictionary);

    cv::Mat boardImage;
    board -> draw(image_size, boardImage, m_marker_separation, m_board_bits);
    m_board = board;
    cv::String output_folder = "../board/board.jpg";
    cv::imwrite(output_folder, boardImage);
}


void Ar::camera_cablirate(std::vector<std::string> file_names, bool refine_detection)
{
    std::vector<cv::Mat> rvecs, tvecs;
    
    int calibration_flags = 0;

    cv::Ptr<cv::aruco::DetectorParameters> detectorParams = cv::aruco::DetectorParameters::create();
    // Read input data
    for (int i = 0; i < file_names.size(); i++)
    {
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners, rejected;
        m_input_images.push_back(cv::imread(file_names[i]));
        // detect markers and detect corners
        cv::aruco::detectMarkers(m_input_images[i], m_dictionary, corners, ids);
        if (refine_detection)
        {
            cv::aruco::refineDetectedMarkers(m_input_images[i], m_board, corners, ids, rejected);
        }
        m_allCorners.push_back(corners);
        m_allIds.push_back(ids);
    }
    
    // Prepare data for calibration
    std::vector<std::vector<cv::Point2f>> all_corners_concatenated;
    std::vector<int> all_ids_concatenated;
    std::vector<int> marker_counter_per_frame;

    marker_counter_per_frame.reserve(m_allIds.size());
    for (int i = 0; i < m_allCorners.size(); i++)
    {
        marker_counter_per_frame.push_back(m_allIds[i].size());
        for (int j = 0; j < m_allCorners[i].size(); j++)
        {
            all_corners_concatenated.push_back(m_allCorners[i][j]);
            all_ids_concatenated.push_back(m_allIds[i][j]);
        }
    }

    // cabilirate camera
    double repError = cv::aruco::calibrateCameraAruco(all_corners_concatenated, all_ids_concatenated,
                                                marker_counter_per_frame, m_board, m_input_images[0].size(), m_camera_matrix, 
                                                m_dist_coeffs, rvecs, tvecs, calibration_flags);
    std::cout << m_camera_matrix << std::endl;
}


void Ar::pose_estimation()
{
    cv::Mat image; 

    m_allRTs.clear();
    m_allRTs.resize(m_input_images.size());
    for (int i = 0; i < m_input_images.size(); i++)
    {
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        image = m_input_images[i];
        cv::aruco::detectMarkers(image, m_dictionary, corners, ids);
        cv::Vec3d rvecs, tvecs;
        cv::aruco::estimatePoseBoard(corners, ids, m_board, m_camera_matrix, m_dist_coeffs, rvecs, tvecs);
        
        // Draw camera's posture calculated from grid board
        cv::Mat R_cam_to_grid, R_grid_to_cam, T_grid_to_cam, RT;
        cv::Rodrigues(rvecs, R_cam_to_grid);
        cv::transpose(R_cam_to_grid, R_grid_to_cam);
        T_grid_to_cam = cv::Mat(3, 1, CV_64F, tvecs);
        
        cv::Mat RT_i(3, 4, CV_64F);
        R_grid_to_cam.copyTo(RT_i.rowRange(0, 3).colRange(0, 3));
        T_grid_to_cam.copyTo(RT_i.rowRange(0, 3).col(3));
        m_allRTs[i] = RT_i;
        std::cout << "RT_" << i + 1 << " is " << std::endl;
        std::cout << RT_i << std::endl;
    }
}


void Ar::background_extraction()
{
    m_extracted_imgs.clear();
    m_extracted_imgs.resize(m_input_images.size());
    for (int i=0; i < m_input_images.size(); i++)
    {
        
        cv::Mat temp = m_input_images[i].clone();
        cv::rectangle(temp, m_boundingBox_paste,  cv::Scalar(0, 255, 0), 3);
        // showImg("test", temp);

        cv::Mat mask = cv::Mat::zeros(m_input_images[i].rows, m_input_images[i].cols, CV_8UC1);
        cv::Mat bgModel, fgModel;
        unsigned int iteration = 5;
        cv::grabCut(m_input_images[i], mask, m_boundingBox_paste, bgModel, fgModel, iteration, cv::GC_INIT_WITH_RECT);
        cv::Mat mask2 = (mask == 1) + (mask == 3);  // 0 = cv::GC_BGD, 1 = cv::GC_FGD, 2 = cv::PR_BGD, 3 = cv::GC_PR_FGD
        cv::Mat dest;
        m_input_images[i].copyTo(dest, mask2);
        std::cout<<"Writing image "<<i + 1<<std::endl;
        cv::imwrite("../Extracted_image/image" + std::to_string(i + 1) + ".jpg", dest);
        m_extracted_imgs[i] = dest;
        
    }
}


void Ar::showImg(const std::string& name,cv::Mat& img)
{
    // test the position of bounding box for background extraction
    cv::namedWindow(name, cv::WINDOW_KEEPRATIO);
    cv::imshow(name,img);
    cv::waitKey(0);
}

void Ar::voxel_carving()
{
    for (int i = 0; i < m_extracted_imgs.size(); i++)
    {
        cv::Mat gray, silhouette;
        cv::Mat project_matrix;
        cv::cvtColor(m_extracted_imgs[i], gray, cv::COLOR_BGR2GRAY);
        cv::threshold(gray, silhouette, 100, 255, cv::THRESH_BINARY);
        project_matrix = m_camera_matrix * m_allRTs[i];

        m_gray.push_back(gray);
        m_silhouette.push_back(silhouette);
        m_project_matrixes.push_back(project_matrix);
    }

    float xmin = -6.21639, ymin = -10.2796, zmin = -14.0349;
    float xmax = 7.62138, ymax = 12.1731, zmax = 10.5358;

    float bbwidth = abs(xmax-xmin)*1.15;
    float bbheight = abs(ymax-ymin)*1.15;
    float bbdepth = abs(zmax-zmin)*1.05;

    params.startX = xmin-abs(xmax-xmin)*0.15;
    params.startY = ymin-abs(ymax-ymin)*0.15;
    params.startZ = 0.0f;
    params.voxelWidth = bbwidth/VOXEL_DIM;
    params.voxelHeight = bbheight/VOXEL_DIM;
    params.voxelDepth = bbdepth/VOXEL_DIM;

    std::cout << "x: " <<params.voxelWidth << std::endl;
    std::cout << "y: " <<params.voxelHeight << std::endl;
    std::cout << "z: " <<params.voxelDepth << std::endl;

    /* 3 dimensional voxel grid */

    int *fArray = new int[VOXEL_SIZE];
    std::fill_n(fArray, VOXEL_SIZE, 1);

    // run carving
    for (int img =0; img < 3; img++)
    {
        std::cout << "Processing image_" << img + 1 << "....." << std::endl;
            for (int i=0; i<VOXEL_DIM; i++) 
            {
                for (int j=0; j<VOXEL_DIM; j++)
                {
                    for (int k=0; k<VOXEL_DIM; k++) 
                    {
                    
                        Voxel v;
                        v.xpos = params.startX + i * params.voxelWidth;
                        v.ypos = params.startY + j * params.voxelHeight;
                        v.zpos = params.startZ + k * params.voxelDepth;
                        
                        Coord im = project(m_project_matrixes[img], v);

                        if (im.x > 0 && im.y > 0 && im.x < IMG_WIDTH && im.y < IMG_HEIGHT) 
                        {
                            if (m_silhouette[img].at<uchar>(im.y, im.x) == BACKGROUND)
                            {
                                fArray[i*VOXEL_SLICE+j*VOXEL_DIM+k] = 0;
                            }
                        }
                    }
                }
            }
    }

    // save points 
    std::ofstream ofs;
    ofs.open("../result_points/points.pts", std::ios::out);

    for (int i=0; i<VOXEL_DIM; i++) 
    {
        for (int j=0; j<VOXEL_DIM; j++)
        {
            for (int k=0; k<VOXEL_DIM; k++) 
            {
                
                
                if (fArray[i*VOXEL_SLICE+j*VOXEL_DIM+k] == 1 )
                {
                    float xpos = params.startX + i * params.voxelWidth;
                    float ypos = params.startY + j * params.voxelHeight;
                    float zpos = params.startZ + k * params.voxelDepth;
            
                    ofs << xpos << " " << ypos << " " << zpos << std::endl;
                }
                
            }
        }
    }
    ofs.close();

    // save silhoutte
    int index = 0;
    for(std::vector<cv::Mat>::iterator iter = m_silhouette.begin(); iter!= m_silhouette.end(); iter++)
    {
        std::cout << index + 1 << std::endl;
        cv::imwrite("../Silhouette/silhouette_"+std::to_string(index + 1)+".jpg", *iter );
        index++;
    }
}

Coord Ar::project(cv::Mat projMatrix, Voxel v) 
{
    
    Coord im;
    
    /* project voxel into camera image coords */
    float z =   projMatrix.at<float>(2, 0) * v.xpos +
                projMatrix.at<float>(2, 1) * v.ypos +
                projMatrix.at<float>(2, 2) * v.zpos +
                projMatrix.at<float>(2, 3);
    
    im.y =    (projMatrix.at<float>(1, 0) * v.xpos +
               projMatrix.at<float>(1, 1) * v.ypos +
               projMatrix.at<float>(1, 2) * v.zpos +
                projMatrix.at<float>(1, 3)) / z;
    
    im.x =    (projMatrix.at<float>(0, 0) * v.xpos +
               projMatrix.at<float>(0, 1) * v.ypos +
               projMatrix.at<float>(0, 2) * v.zpos +
                projMatrix.at<float>(0, 3)) / z;
    
    return im;
}