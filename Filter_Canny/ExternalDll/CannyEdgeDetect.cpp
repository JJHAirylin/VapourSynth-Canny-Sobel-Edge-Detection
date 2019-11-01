
#include <stdlib.h>
#include <string.h>
#include "stdafx.h"
#include "ExternalDll.h"
#include<opencv2/opencv.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>


extern "C" __declspec(dllexport) 
void CannyEdgeDetect(unsigned char* yuvIn, int w, int h, unsigned char* yuvOut) {
		
		int bufLen = w*h * 3 / 2; //���鳤��
		
		cv::Mat yuvImg;
		yuvImg.create(h * 3 / 2, w, CV_8UC1); //��ͨ��yuv��ʽͼ��
		memcpy(yuvImg.data, yuvIn, bufLen * sizeof(unsigned char));
		cv::Mat blurImg, canny_output;
		blur(yuvImg, blurImg, cv::Size(7, 7));    //�˲�
		Canny(blurImg, canny_output, 20, 50, 3);  //canny��Ե���
		memcpy(yuvOut, canny_output.data, bufLen * sizeof(unsigned char));  //���
		
	}






