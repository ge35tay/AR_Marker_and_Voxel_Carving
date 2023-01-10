#include <iostream>
#include <sstream>
#include <filesystem>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <opencv2/opencv.hpp>

const char * params 
    = "{ video | 0 | video stream as input}"
    "{ image | 0 | pre saved image as input}"
    "{ algo  | MOG2 | Background subtraction method (KNN, MOG2) }";

cv::Mat sub_back_ground(const cv::Mat& img, cv::Ptr<cv::BackgroundSubtractor> pBackSub, cv::Mat & fgmask)
{   
    // get front mask
    pBackSub->apply(img, fgmask);

    // // eliminate the noise
    // cv::Mat line = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 5), cv::Point(-1, -1));
    // cv::morphologyEx(fgmask, fgmask, cv::MORPH_OPEN, line);
    return fgmask;
}
int main(int argc, char* argv[])
{
    cv::CommandLineParser parser(argc, argv, params);
    cv::Ptr<cv::BackgroundSubtractor> pBackSub;


    if (parser.get<cv::String>("algo") == "MOG2")
    {
        pBackSub = cv::createBackgroundSubtractorMOG2(5, 200);
    }
    else
    {
        pBackSub = cv::createBackgroundSubtractorKNN(5, 200);
    }

    if (parser.get<cv::String>("video") == "1")
    {
        cv::VideoCapture in_video;
        cv::String video_input = "2";
        char* end = nullptr;
        int source = static_cast<int>(std::strtol(video_input.c_str(), &end, \
            10));
        if (!end || end == video_input.c_str()) {
            in_video.open(video_input); // url
        } else {
            in_video.open(source); // id
        }

        cv::Mat frame, foreground_mask;
        while(in_video.isOpened() && in_video.read(frame))
        {
            foreground_mask = sub_back_ground(frame, pBackSub, foreground_mask);

            // show the current frame and fgmasks
            cv::imshow("Frame", frame);
            cv::imshow("Foreground", foreground_mask);
            if (cv::waitKey(10) == 27)
            {
                break;
            }
        }
        in_video.release();
    }
    else if(parser.get<cv::String>("image") == "1")
    {
        std::vector<std::string> images;
        cv::glob("../../source/Back_ground/image*.jpg", images);
        unsigned x = 1;

        for (const auto &img :images)
        {
            cv::Mat image = cv::imread(img);
            cv::Mat foreground_mask;
            foreground_mask = sub_back_ground(image, pBackSub, foreground_mask);
            cv::imwrite(std::string("../result/") + "mask_" + std::to_string(x) + ".jpg", foreground_mask);
            x++;
        }
    }
    return 0;
}