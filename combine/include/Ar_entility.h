#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <fstream>
#include <sstream>

extern const int IMG_WIDTH;
extern const int IMG_HEIGHT;
extern const int VOXEL_DIM;
extern const int VOXEL_SIZE;
extern const int VOXEL_SLICE;
extern const int BACKGROUND;


struct Voxel {
    float xpos;
    float ypos;
    float zpos;
};

struct Coord {
    int x;
    int y;
};

struct StartParams {
    float startX;
    float startY;
    float startZ;
    float voxelWidth;
    float voxelHeight;
    float voxelDepth;
};


class Ar
{
public:
    void generate_arucoboard(int markerX, int markerY, int marker_length, int marker_separation, int dictionary_id, int board_bits);
    void camera_cablirate(std::vector<std::string> file_names, bool refine_detection);
    void pose_estimation();
    void background_extraction();
    void showImg(const std::string& name, cv::Mat& img);
    void voxel_carving();
    Coord project(cv::Mat projMatrix, Voxel v);

private:
    // Generation of markers
    // Need to be printed first
    int m_markerX;
    int m_markerY;
    int m_marker_length;
    int m_marker_separation;
    int m_dictionary_id;
    cv::Ptr<cv::aruco::Dictionary> m_dictionary;
    int m_board_bits;
    cv::Ptr<cv::aruco::GridBoard> m_board;

    // Camera Calibrate
    std::vector<cv::Mat> m_input_images;
    std::vector<std::vector<std::vector<cv::Point2f>>> m_allCorners;
    std::vector<std::vector<int>> m_allIds;
    std::vector<cv::Mat> m_allRTs;
    cv::Mat m_camera_matrix;
    cv::Mat m_dist_coeffs;

    // Background extraction
    cv::Rect m_boundingBox_paste = {800, 400, 2800, 1600};
    cv::Rect m_boundingBox_Airpod = {800, 400, 2800, 1600};
    std::vector<cv::Mat> m_extracted_imgs;

    // Voxel Carving
    std::vector<cv::Mat> m_gray;
    std::vector<cv::Mat> m_silhouette;
    std::vector<cv::Mat> m_project_matrixes;
};