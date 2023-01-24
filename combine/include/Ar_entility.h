#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <string>
class Ar
{
public:
    void generate_arucoboard(int markerX, int markerY, int marker_length, int marker_separation, int dictionary_id, int board_bits);
    void camera_cablirate(std::vector<std::string> file_names, bool refine_detection);
private:
    // Generation of markers
    // Need to print first
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
    cv::Mat m_camera_matrix;
    cv::Mat m_dist_coeffs;
};