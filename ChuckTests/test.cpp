#include <libfreenect.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <cv.h>
#include <cxcore.h>
//#include <opencv/highgui.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;


class myMutex {
public:
	myMutex() {
		pthread_mutex_init( &m_mutex, NULL );
	}
	void lock() {
		pthread_mutex_lock( &m_mutex );
	}
	void unlock() {
		pthread_mutex_unlock( &m_mutex );
	}
private:
	pthread_mutex_t m_mutex;
};


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

//TODO: Have you considered using masks other chuck? why yesi, yes i have chuck.
Mat findPath(Mat depthMat,Mat pathMat)
{
	int scanDepth = 1;
	int scanResolution = 2;
	//cout << depthMat << endl;
	int numChannels = depthMat.channels();
	//cout << numChannels << endl;
	transpose(depthMat, depthMat);
	flip(depthMat,depthMat,1);

	transpose(pathMat, pathMat);
	flip(pathMat,pathMat,1);
	int i;
	for(i=1; i<depthMat.rows;i+=scanResolution){
		uchar* rowi = depthMat.ptr(i);
		int j;
		if(i==321)
		{
			cout << "start of data" << endl;
		}
		for( j=1; j<depthMat.cols-scanDepth; j+=numChannels){
			int diff =rowi[j+scanDepth]-rowi[j];
			/*
			if (diff < 1 && diff > -3)
			{
				rowi[j] = 0;
			}
			*/
			//rowi[j]=j/2;
			if (i==321 && j < 100)
			{
				cout <<(int) rowi[j] << endl;
				rowi[j]=0;

			}
		}
		//cout << j << endl;
	}
	flip(depthMat,depthMat,1);
	transpose(depthMat,depthMat);
	return depthMat;
	/*
	transpose(depthMat, rotated);
	flip(rotated,rotated,1);
	cout << rotated.size() << endl;
	return rotated;
	*/
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
		depthPathMat = depthf.clone();
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

