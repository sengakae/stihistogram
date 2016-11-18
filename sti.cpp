#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "math.h"

#include <iostream>
#include <string>

using namespace cv;
using namespace std;

Mat resizeFrame(Mat frame) {
	Size size(32, 32);
	return resize(frame, res, size);
}

Mat chromaticityConv(Mat frame) {
	Mat res(2, frame.type());
	for(int i = 0; i < frame.rows; ++i) {
		for(int j = 0; j < frame.cols; ++j) {
			Vec3f intensity = frame.at<Vec3b>(i,j);
			float blue = intensity.val[0];
			float green = intensity.val[1];
			float red = intensity.val[2];
			
			float sum = red + blue + green;
			float r = red / sum;
			float g = green / sum;
			
			res.data[res.step[0]*i + res.step[1]*j + 0] = (g * 255);
			res.data[res.step[0]*i + res.step[1]*j + 1] = (r * 255);
		}
	}
}

int main(int argc, char** argv) {
	// reads video file as argument
	if(argc < 2) {
		cout << "Usage: ./program <filename>" << endl;
		return -1;
	}
		
	cout << "Filename: " << argv[1] << endl;
	
	Mat frame;
	VideoCapture capture;
	if(capture.open(argv[1])) {
		double videoFPS = capture.get(CV_CAP_PROP_FPS);
		int nbFrames = capture.get(CV_CAP_PROP_FRAME_COUNT);
		for(int i = 0; i < nbFrames; ++i) {
			capture >> frame;
			// Do stuff for frames
			resizeFrame(frame);
			
		}
	else {
		cout << "File not found" << endl;
	}
}