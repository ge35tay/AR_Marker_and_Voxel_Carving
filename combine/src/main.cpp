#include <iostream>
#include "Ar_entility.h"

int main()
{
    // TODO too many input argument, need a config file
    Ar ar_exp;
    int marker_x = 4;
    int marker_y = 4;
    int marker_length = 200;
    int marker_separation = 100;
    int dictionary_id = 256;
    int board_bits = 1;

    std::vector<std::string> filenames;
    filenames.push_back("../Images/Airpod/1.jpg");
    filenames.push_back("../Images/Airpod/2.jpg");
    filenames.push_back("../Images/Airpod/3.jpg");
    bool refine = true;
    ar_exp.generate_arucoboard(marker_x, marker_y, marker_length, marker_separation, dictionary_id, board_bits);
    ar_exp.camera_cablirate(filenames, refine);
    ar_exp.pose_estimation();
    ar_exp.background_extraction();
    ar_exp.voxel_carving();
    return 0;
}