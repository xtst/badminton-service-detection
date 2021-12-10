#define WINDOW_NAME "CVUI Hello World!"
#define CVUI_IMPLEMENTATION


#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2\imgproc\types_c.h>
#include <opencv2/video/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <Windows.h>
#include <stdio.h>
#include <ctime>
#include "cvui.h"
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<chrono>
using namespace std;
using namespace cv;

void flip(Mat& image)
{
    cvtColor(image, image, CV_BGR2GRAY);
    int nr = image.rows;                    // number of rows
    int nc = image.cols * image.channels(); // total number of elements per line
    for (int j = 0; j < nr; j++)
    {
        uchar* data = image.ptr<uchar>(j);
        for (int i = 0; i < nc; i++)
        {
            if (data[i] > 210)
                data[i] = 255;
            else
                data[i] = 0;
        }
    }
}

void find(Mat& image, pair<int, int>& pos)
{
    flip(image);
    int nr = image.rows;                    // number of rows
    int nc = image.cols * image.channels(); // total number of elements per line
    int all = 0;
    for (int j = nr / 5; j < nr / 5 * 4; j++)
    {
        uchar* data = image.ptr<uchar>(j);
        for (int i = nc / 3; i < nc / 3 * 2; i++)
        {
            if (data[i] > 210)
            {
                all++;
                pos.first += i;
                pos.second += j;
            }
        }
    }
    if (all == 0)
        return;
    pos.first /= all;
    pos.second /= all;
}

int main()
{
    // cv::namedWindow(WINDOW_NAME);
    cvui::init(WINDOW_NAME);

    
    cv::utils::logging::setLogLevel(utils::logging::LOG_LEVEL_SILENT);
    system("color F0");
    Mat frame, gray, frameDelta, thresh, firstFrame;
    vector<vector<Point>> cnts;
    VideoCapture camera(0); // open camera
 //   camera.set(3, 512);
  //  camera.set(4, 288);

    if (camera.isOpened())
    {
        cout << "图像宽度=" << camera.get(CAP_PROP_FRAME_WIDTH) << endl;
        cout << "图像高度=" << camera.get(CAP_PROP_FRAME_HEIGHT) << endl;
        cout << "视频帧率=" << camera.get(CAP_PROP_FPS) << endl;
        cout << "总帧数=" << camera.get(CAP_PROP_FRAME_COUNT) << endl;
    }
    else
    {
        cout << "视频名称不正确" << endl;
        return -1;
    }


    // set the video size to 512x288 to process faster

    Sleep(3);

    camera.read(frame);
    int past_time = time(NULL);
    // convert to grayscale and set the first frame
    cvtColor(frame, firstFrame, COLOR_BGR2GRAY);
    GaussianBlur(firstFrame, firstFrame, Size(21, 21), 0);

    while (camera.read(frame))
    {
        if (cvui::button(frame, 110, 80, "Hello, world!")) {
            // 计数
            putText(frame, "ok", Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
           // return 0;
        }


        // convert to grayscale
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        GaussianBlur(gray, gray, Size(21, 21), 0);

        // compute difference between first frame and current frame
        absdiff(firstFrame, gray, frameDelta);
        threshold(frameDelta, thresh, 25, 255, THRESH_BINARY);

        dilate(thresh, thresh, Mat(), Point(-1, -1), 2);
        findContours(thresh, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        bool flag = 0;
        Mat past = frame;
        pair<int, int> pos;
        for (int i = 0; i < cnts.size(); i++)
        {
            if (contourArea(cnts[i]) < 7500)
            {
                past = frame;
                continue;
            }
            if (flag == 0)
            {
                find(past, pos);
            }
            //  imshow("show", past);
            flag = 1;
            string output = "";
            if (pos.second < frame.rows / 2) {
                output = "Service Faults";
            }
            else {
                output = "Legal Service";
            }
            output += " (" + to_string(pos.first) + "," + to_string(pos.second) + ")";
            putText(frame, output, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);

        }

        imshow("Camera", frame);

        if (waitKey(1) == 27)
        {
            // exit if ESC is pressed
            break;
        }

        // firstFrame = gray;
        int now_time = time(NULL);
        if (now_time - past_time > 2)
        {
            camera.read(frame);

            cvtColor(frame, firstFrame, COLOR_BGR2GRAY);
            GaussianBlur(firstFrame, firstFrame, Size(21, 21), 0);
            past_time = now_time;
        }
        waitKey(30);
    }

    return 0;
}



/*int main()
{
    cv::utils::logging::setLogLevel(utils::logging::LOG_LEVEL_SILENT);
    system("color F0");
    VideoCapture video(0);
    //	video.set(3, 512);
    //	video.set(4, 288);
    // VideoCapture video("service_video.mp4");
    if (video.isOpened())
    {
        cout << "图像宽度=" << video.get(CAP_PROP_FRAME_WIDTH) << endl;
        cout << "图像高度=" << video.get(CAP_PROP_FRAME_HEIGHT) << endl;
        cout << "视频帧率=" << video.get(CAP_PROP_FPS) << endl;
        cout << "总帧数=" << video.get(CAP_PROP_FRAME_COUNT) << endl;
    }
    else
    {
        cout << "视频名称不正确" << endl;
        return -1;
    }

    Mat temp;
    video >> temp;
    pair<int, int> pos(0, 0);
    find(temp, pos);
    // Canny(temp, temp, 220, 240, 3);
    imshow("video", temp);
    cout << "中心点对" << pos.first << " " << pos.second << endl;

    Mat edges;
    waitKey(3000);
    if (1)
    {
        Mat frame;
        video >> frame;
        cvtColor(frame, edges, CV_BGR2GRAY); //变为灰度图
                                             //	blur(edges, edges, Size(3, 3));//均值滤波降噪
        Mat grey = edges;
        //	Canny(edges, edges, 80, 120, 3);//canny算子
        if (frame.empty())
        {
            // break;
        }
        imshow("video", edges);
        waitKey(1000 / video.get(CAP_PROP_FPS));
        // waitKey(30);
    }
    waitKey();
    return 0;
}*/
