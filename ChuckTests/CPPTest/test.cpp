#include <iostream>
#include <libfreenect.hpp>
#include <cv.h>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <cv.h>
#include <cxcore.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "myMutex.hpp"

using namespace cv;
using namespace std;

class MyFreenectDevice : public Freenect::FreenectDevice {
public:
	MyFreenectDevice(freenect_context *_ctx, int _index)
		: Freenect::FreenectDevice(_ctx, _index), m_buffer_depth(FREENECT_DEPTH_11BIT),
		m_buffer_rgb(FREENECT_VIDEO_RGB), m_gamma(2048), m_new_rgb_frame(false),
		m_new_depth_frame(false), depthMat(Size(640,480),CV_16UC1),
		rgbMat(Size(640,480), CV_8UC3, Scalar(0)),
		ownMat(Size(640,480),CV_8UC3,Scalar(0)) {

			for( unsigned int i = 0 ; i < 2048 ; i++) {
				float v = i/2048.0;
				v = std::pow(v, 3)* 6;
				m_gamma[i] = v*6*256;
			}
		}

	// Do not call directly even in child
	void VideoCallback(void* _rgb, uint32_t timestamp) {
		//std::cout << "RGB callback" << std::endl;
		m_rgb_mutex.lock();
		uint8_t* rgb = static_cast<uint8_t*>(_rgb);
		rgbMat.data = rgb;
		m_new_rgb_frame = true;
		m_rgb_mutex.unlock();
	};

	// Do not call directly even in child
	void DepthCallback(void* _depth, uint32_t timestamp) {
		//std::cout << "Depth callback" << std::endl;
		m_depth_mutex.lock();
		uint16_t* depth = static_cast<uint16_t*>(_depth);
		depthMat.data = (uchar*) depth;
		m_new_depth_frame = true;
		m_depth_mutex.unlock();
	}

	bool getVideo(Mat& output) {
		m_rgb_mutex.lock();
		if(m_new_rgb_frame) {
			cv::cvtColor(rgbMat, output, CV_RGB2BGR);
			m_new_rgb_frame = false;
			m_rgb_mutex.unlock();
			return true;
		} else {
			m_rgb_mutex.unlock();
			return false;
		}
	}

	bool getDepth(Mat& output) {
		m_depth_mutex.lock();
		if(m_new_depth_frame) {
			depthMat.copyTo(output);
			m_new_depth_frame = false;
			m_depth_mutex.unlock();
			return true;
		} else {
			m_depth_mutex.unlock();
			return false;
		}
	}
private:
	std::vector<uint8_t> m_buffer_depth;
	std::vector<uint8_t> m_buffer_rgb;
	std::vector<uint16_t> m_gamma;
	Mat depthMat;
	Mat rgbMat;
	Mat ownMat;
	myMutex m_rgb_mutex;
	myMutex m_depth_mutex;
	bool m_new_rgb_frame;
	bool m_new_depth_frame;
};

//TODO: Have you considered using masks other chuck? why yes i, yes i have chuck.
// Make it so that the alg doesnt change a Mat (something we cant undo easy)
// Instead have the mask set so we can manipulate 1s and 0s in it for what we
// want. Then do something like
// (mask&&depth) to get th mapable drivable area and similar for obsticals
//
//Though maybe not logarithms (xkcd joke).
//TODO: add this to the cppview.cpp so we can see how it looks in the three
//dimensional view.
//
//TODO:
//Consider storing this as a 1xN matrix where the elements are the max depth
//to seen acceptable path
//
//Then the next thing to do is to push that  into something that shows the max
//distance it can travel in that direction based on lense... arc angle something
//
//TODO:
//Consider a pendulum with this it swings down to a point where there is an arc
//that moves it back. try to find the minimum. hey look numerical analysis. i
//knew this math would matter at some point.

Mat findPath(Mat depthMat,Mat pathMat)
{
	int scanDepth = 5;
	int skipTolerance = 3;
	int i;
	for(i=1; i<depthMat.cols;i++)
	{
		int numSkips = 0;
		int j;
		for( j=depthMat.rows-2; j>0 /*depthMat.rows-scanDepth*/; j--)
		{
			int difference = (int) depthMat.at<unsigned short>(j,i)
				- (int)depthMat.at<unsigned short>(j-1,i);
			//cout << (int)pathMat.at<uchar>(i,j) << endl;
			if (difference<=0 && (int) depthMat.at<unsigned short>(j,i)!=2047)
			{
				//TODO: update this so that when it gets done it
				//goes back and doesnt have passable as under
				//overhangs.
				//depthMat.at<unsigned short>(j,i)=0;
				//remember to uncomment the conversation
				//at the bottom after this
				pathMat.at<uchar>(j,3*i)=255;
				pathMat.at<uchar>(j,3*i+1)=0;
				pathMat.at<uchar>(j,3*i+2)=0;
			}
			else
			{
				numSkips++;
			}

			if(numSkips>skipTolerance)
			{
				break;
			}
		}
	}
	return pathMat;
	//return depthMat;
}

//TODO: change this so that it uses masks
void findHSV(Mat hsvMat, int hueLow, int hueHigh)
{
	int numChannels = 3;
	for(int i=0; i<hsvMat.rows;i++){
		uchar* rowi = hsvMat.ptr(i);
		for(int j=0; j<hsvMat.cols*numChannels; j+=numChannels){
			if (rowi[j]<hueLow || rowi[j] > hueHigh || rowi[j+1]<255/2)
			{
				for(int k = 0; k<numChannels;k++){
					rowi[j+k]=0;
				}
			}
		}
	}
}

int main(int argc, char **argv) {
	bool die(false);
	string filename("snapshot");
	string suffix(".png");
	int i_snap(0),iter(0);
	int scanRate = 5;
	Mat depthMat(Size(640,480),CV_16UC1);
	Mat depthf (Size(640,480),CV_8UC1);
	Mat rgbMat(Size(640,480),CV_8UC3,Scalar(0));
	Mat pathMat;
	Mat ownMat(Size(640,480),CV_8UC3,Scalar(0));
	Mat hsvMat;
	Mat depthPathMat;
	// The next two lines must be changed as Freenect::Freenect
	// isn't a template but the method createDevice:
	// Freenect::Freenect<MyFreenectDevice> freenect;
	// MyFreenectDevice& device = freenect.createDevice(0);
	// by these two lines:

	Freenect::Freenect freenect;
	MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);

	namedWindow("rgb",CV_WINDOW_AUTOSIZE);
	namedWindow("depth",CV_WINDOW_AUTOSIZE);
	namedWindow("hsv",CV_WINDOW_AUTOSIZE);
	namedWindow("path",CV_WINDOW_AUTOSIZE);

	device.startVideo();
	device.startDepth();
	while (!die) {
		device.getVideo(rgbMat);
		device.getDepth(depthMat);


		cvtColor(rgbMat,hsvMat, CV_BGR2HSV);
		findHSV(hsvMat, 220/2,260/2);
		cvtColor(hsvMat,hsvMat, CV_HSV2BGR);
		depthMat.convertTo(depthf, CV_8UC1, 255.0/2048.0);

		//device.lockAll();
		//cout << "test" << endl;
		depthPathMat = depthMat.clone();
		//cout << depthPathMat << endl;
		pathMat = rgbMat.clone();
		pathMat = findPath(depthPathMat,pathMat);
		//pathMat.convertTo(pathMat, CV_8UC1, 255.0/2048.0);

		//device.unlockAll();

		imshow("depth",depthf);
		imshow("hsv",hsvMat);
		imshow("rgb", rgbMat);
		imshow("path",pathMat);
		char k = cvWaitKey(5);
		if( k == 27 ){
			cvDestroyWindow("rgb");
			cvDestroyWindow("depth");
			break;
		}
		if( k == 8 ) {
			std::ostringstream file;
			file << filename << i_snap << suffix;
			cv::imwrite(file.str(),rgbMat);
			i_snap++;
		}
	}

	device.stopVideo();
	device.stopDepth();
	return 0;
}



