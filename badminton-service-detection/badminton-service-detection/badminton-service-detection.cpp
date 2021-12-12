#define WINDOW_NAME "CVUI Hello World!"
#define CVUI_IMPLEMENTATION
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "WINMM.LIB")
#pragma
//#include "cvui.h"
#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <dsound.h>
#include <fstream>
#include <iostream>
#include <mmsystem.h>
#include <opencv2/core/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2\imgproc\types_c.h>
#include <stdio.h>
#include <windows.h>
using namespace std;
using namespace cv;

void flip(Mat& image) {
	cvtColor(image, image, CV_BGR2GRAY);
	int nr = image.rows;					// number of rows
	int nc = image.cols * image.channels(); // total number of elements per line
	for (int j = 0; j < nr; j++) {
		uchar* data = image.ptr<uchar>(j);
		for (int i = 0; i < nc; i++) {
			if (data[i] > 215)
				data[i] = 255;
			else
				data[i] = 0;
		}
	}
}

void find(Mat& image, pair<int, int>& pos, int& pointNum, Mat& past1) {
	flip(image);
	int nr = image.rows;					// number of rows
	int nc = image.cols * image.channels(); // total number of elements per line
	int all = 0;
	for (int j = nr / 5; j < nr / 5 * 4; j++) {
		uchar* data = image.ptr<uchar>(j);
		uchar* data2 = past1.ptr<uchar>(j);
		for (int i = nc / 5; i < nc / 5 * 4; i++) {
			if (data[i] > 215 && data2[i]!=data[i]) {
				all++;
				pos.first += i;
				pos.second += j;
			}
		}
	}
	if (all == 0) return;
	pos.first /= all;
	pos.second /= all;
	pointNum = all;
}

void showInstruction(Mat frame) {
	int row = max(frame.rows, 800);
	int col = max(frame.cols, 1280);
	//创建空白图用于绘制文字
	cv::Mat image = cv::Mat::zeros(cv::Size(col, row), CV_8UC3);
	//设置蓝色背景
	image.setTo(cv::Scalar(237, 237, 237));

	//设置绘制文本的相关参数
	std::string text = "Enter ESC to quit. Enter any key to continue.";
	int font_face = cv::FONT_HERSHEY_SIMPLEX;
	double font_scale = 1.70;
	int thickness = 2;
	int baseline;
	//获取文本框的长宽
	cv::Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);

	//将文本框居中绘制
	cv::Point origin;
	origin.x = image.cols / 2 - text_size.width / 2;
	origin.y = image.rows / 2 + text_size.height / 2;
	cv::putText(image, text, origin, font_face, font_scale, cv::Scalar(23, 23, 23), thickness, 4, 0);

	//显示绘制解果
	cv::imshow("羽毛球发球检测系统[摄像头]（按ESC退出）", image);
	if (waitKey(0) == 27) {
		// exit if ESC is pressed
		exit(0);
	}
	// cv::waitKey(0);
}

string FilePosition = "default.bsd";
string MusicPosition = "service-too-high.mp3";
int CameraNumber = 0, TimeBetweenService = 2, TimeBetweenFrame = 30, MoveRange = 25;

void processArguments() {
	ifstream infile("default.bsd"); // bsd stands for badminton detection system
	string key, value;
	while (infile >> key >> value) {
		cout << key << " " << value << endl;
		if (key == "FilePosition:") {
			ifstream infile2(value);
			//infile2.close();
			while (infile2 >> key >> value) {
				cout << key << " " << value << endl;
				if (key == "MusicPosition:") {
					MusicPosition = value;
					// Play Music 1
				}
				if (key == "TimeBetweenService:") { TimeBetweenService = stoi(value); }
				if (key == "TimeBetweenFrame:") { TimeBetweenFrame = stoi(value); }
				if (key == "MoveRange:") { MoveRange = stoi(value); }
				if (key == "CameraNumber:") { CameraNumber = stoi(value); }
			}
			infile2.close();
			return;
		}
		if (key == "MusicPosition:") {
			MusicPosition = value;
			// Play Music 1
		}
		if (key == "TimeBetweenService:") { TimeBetweenService = stoi(value); }
		if (key == "TimeBetweenFrame:") { TimeBetweenFrame = stoi(value); }
		if (key == "MoveRange:") { MoveRange = stoi(value); }
		if (key == "CameraNumber:") { CameraNumber = stoi(value); }
	}
	infile.close();
}

int main() {
	// cv::namedWindow(WINDOW_NAME);
	// cvui::init(WINDOW_NAME);

	// mciSendString(L("play service-too-high.mp3 wait"), NULL, 0, NULL);
	processArguments();

	cv::utils::logging::setLogLevel(utils::logging::LOG_LEVEL_SILENT);
	system("color F0");
	Mat frame, gray, frameDelta, thresh, firstFrame;
	vector<vector<Point>> cnts;
	VideoCapture camera(CameraNumber); // open camera //   camera.set(3, 512); //  camera.set(4, 288);

	if (camera.isOpened()) {
		cout << "图像宽度=" << camera.get(CAP_PROP_FRAME_WIDTH) << endl;
		cout << "图像高度=" << camera.get(CAP_PROP_FRAME_HEIGHT) << endl;
		cout << "视频帧率=" << camera.get(CAP_PROP_FPS) << endl;
		cout << "总帧数=" << camera.get(CAP_PROP_FRAME_COUNT) << endl;
	}
	else {
		cout << "视频名称不正确" << endl;
		return -1;
	}

	// set the video size to 512x288 to process faster
	// showInstruction();

	camera.read(frame);
	// showInstruction(frame);

	int past_time = time(NULL), pastPos = -1;
	// convert to grayscale and set the first frame
	cvtColor(frame, firstFrame, COLOR_BGR2GRAY);
	GaussianBlur(firstFrame, firstFrame, Size(21, 21), 0);

	Mat past1 = frame;
	waitKey(TimeBetweenFrame);
	while (camera.read(frame)) {
		// convert to grayscale
		cvtColor(frame, gray, COLOR_BGR2GRAY);
		GaussianBlur(gray, gray, Size(21, 21), 0);

		// compute difference between first frame and current frame
		absdiff(firstFrame, gray, frameDelta);
		threshold(frameDelta, thresh, 25, 255, THRESH_BINARY);

		dilate(thresh, thresh, Mat(), Point(-1, -1), 2);
		findContours(thresh, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		bool flag = 0;
		pair<int, int> pos;
		Mat past = frame;
		for (int i = 0; i < cnts.size(); i++) {
			if (contourArea(cnts[i]) < 2800) {
				past = frame;
				continue;
			}
			int pointNumber = 0;
			int MaxNum = past.cols * past.rows / 10;
			int MinNum = past.cols * past.rows / 12000;
			if (flag == 0) {
				find(past, pos, pointNumber, past1);
				if (pointNumber > MaxNum) {
					putText(frame, (string)("Background is too white."), Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 162, 255), 2);
					// cout << "MaxNUM  " << MaxNum  << endl;
					break;
				}
			}
			//  imshow("show", past);
			flag = 1;
			string output = "";
			auto color = Scalar(0, 0, 255);
			if (pointNumber > MinNum && pastPos > 0 && abs(pos.first - pastPos) > MoveRange) {
				if (pos.second < frame.rows / 2) {
					cout << MusicPosition << endl;
					output = "Service Faults";
					string s = "play " + MusicPosition; /* +" wait";*/
					// get temporary LPSTR (not really safe)
					LPSTR pst = &s[0];
					// get temporary LPCSTR (pretty safe)
					LPCSTR pcstr = s.c_str();
					// convert to std::wstring
					std::wstring ws;
					ws.assign(s.begin(), s.end());
					// get temporary LPWSTR (not really safe)
					LPWSTR pwst = &ws[0];
					// get temporary LPCWSTR (pretty safe)
					LPCWSTR pcwstr = ws.c_str();
					mciSendString(pcwstr, NULL, 0, NULL);

					// std::wstring stemp = std::wstring(MusicPosition.begin(), MusicPosition.end());
					// LPCWSTR sw = stemp.c_str();
					// PlaySound(sw , NULL, SND_FILENAME); //SND_FILENAME or SND_LOOP

					// color = Scalar(0, 255, 126);
				}
				else {

					output = "Legal Service";
					color = Scalar(0, 255, 126);
				}
			}
			else
				continue;
			output += " (" + to_string(pos.first) + "," + to_string(pos.second) + ")";
			putText(frame, output, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.75, color, 2);
			for (int i = 1; i <= 2.2 * TimeBetweenFrame; i++) {
				camera.read(frame);
				putText(frame, output, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.75, color, 2);
				imshow("羽毛球发球检测系统[摄像头]（按ESC退出）", frame);
				waitKey(TimeBetweenFrame);
			}
			//	waitKey(50 * TimeBetweenFrame);
			break;
		}

		pastPos = pos.first;
		imshow("羽毛球发球检测系统[摄像头]（按ESC退出）", frame);

		if (waitKey(3) == 27) {
			// exit if ESC is pressed
			return 0;
		}

		// firstFrame = gray;
		int now_time = time(NULL);
		if (now_time - past_time > TimeBetweenService) {
			camera.read(frame);
			cvtColor(frame, firstFrame, COLOR_BGR2GRAY);
			GaussianBlur(firstFrame, firstFrame, Size(21, 21), 0);
			past_time = now_time;
			past = frame;
		}
		past1 = frame;
		waitKey(TimeBetweenFrame);
	}

	return 0;
}
