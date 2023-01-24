#include <Ar_entility.h>

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
}