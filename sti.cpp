#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "math.h"

#include <iostream>
#include <string>

using namespace cv;
using namespace std;

float makeHist(Mat img1, Mat img2) {
  if(img1.rows != img2.rows) {
    cout << "Error - Row count does not match" << endl;
    exit(1);
  }
    
  int bins = 1 + log(img2.rows)/log(2);

  float range[] = {0, 256};
  const float* histRange = {range};

  vector<Mat> bgr_planes1;
  split(img1, bgr_planes1);

  vector<Mat> bgr_planes2;
  split(img2, bgr_planes2);

  Mat rgHist1, rgHist2;

  vector<Mat> rg_temp1;
  rg_temp1.push_back(bgr_planes1[1]);
  rg_temp1.push_back(bgr_planes1[2]);

  vector<Mat> rg_temp2;
  rg_temp2.push_back(bgr_planes2[1]);
  rg_temp2.push_back(bgr_planes2[2]);

  Mat rg1;
  merge(rg_temp1, rg1);

  Mat rg2;
  merge(rg_temp2, rg2);

  int imgCount = 1;
  int dims = 1;
  const int channels[] = {0, 1};
  calcHist(&rg1, imgCount, channels, Mat(), rgHist1, dims, &bins, &histRange, true, false);
  normalize(rgHist1, rgHist1, 0, 1, NORM_MINMAX, -1, Mat());

  calcHist(&rg2, imgCount, channels, Mat(), rgHist2, dims, &bins, &histRange, true, false);
  normalize(rgHist2, rgHist2, 0, 1, NORM_MINMAX, -1, Mat());
 
  float rg1_sum = 0;
  for(int i = 0; i < bins; ++i) {
    rg1_sum += rgHist1.at<float>(i);
  }

  float rg2_sum = 0;
  for(int j = 0; j < bins; ++j) {
    rg2_sum += rgHist2.at<float>(j);
  }

  for(int k = 0; k < bins; ++k) {
    rgHist1.at<float>(k) = rgHist1.at<float>(k) / rg1_sum;
    rgHist2.at<float>(k) = rgHist2.at<float>(k) / rg2_sum;
  }

  return compareHist(rgHist1, rgHist2, CV_COMP_INTERSECT);
}

Mat chromConv(Mat img) {
  Mat res(img.size(), img.type());
  for (int i = 0; i < img.cols; ++i) {
    for(int j = 0; j < img.rows; ++j) {
      Vec3f intensity = img.at<Vec3b>(j,i);
      float blue = intensity.val[0];
      float green = intensity.val[1];
      float red = intensity.val[2];
      
      double sum = red + blue + green;
      
      double r, g, b;
      
      if(sum == 0) {
	r = 0;
	g = 0;
	b = 0;
      }
      else {
	r = red / sum;
	g = green / sum;
	b = blue / sum;
      }

      res.data[res.step[0]*i + res.step[1]*j + 0] = (b*255);
      res.data[res.step[0]*i + res.step[1]*j + 1] = (g*255);
      res.data[res.step[0]*i + res.step[1]*j + 2] = (r*255);
      
      //cout << res.at<Vec3f>(0,i) << endl;
    }
    //col = img.col(i);
    //cout << col.size() << endl;
  }

  imshow("test", res);
  waitKey(1);

  return res;
}

int main(int argc, char** argv) {
  // reads video file as argument
  if(argc < 2) {
    cout << "Usage: ./program <video>" << endl;
    return -1;
  }
  
  cout << "Filename: " << argv[1] << endl;
  
  Mat frame1, frame2;
  VideoCapture capture;
  if(capture.open(argv[1])) {
    double fps = capture.get(CV_CAP_PROP_FPS);
    int vidframes = capture.get(CV_CAP_PROP_FRAME_COUNT);
    Size size(128, vidframes);
    Mat histImg(size, CV_32F);
    cout << "fps: " << fps <<  endl;
    cout << "total frames: " << vidframes << endl;

    capture.set(CV_CAP_PROP_POS_FRAMES, 0);
    capture >> frame1;
   
    for(int i = 1; i < vidframes - 1; ++i) {
      capture.set(CV_CAP_PROP_POS_FRAMES, i);
      capture >> frame2;
      if(frame1.cols == 0 || frame2.cols == 0) {
	cout << "frame: " << i << endl;
	return -1;
      }

      // convert frames before histogram creation
      resize(frame1, frame1, Size(128, 128));
      Mat chrom1 = chromConv(frame1);
      
      resize(frame2, frame2, Size(128, 128));
      Mat chrom2 = chromConv(frame2);
      
      for(int j = 0; j < frame2.cols; ++j) {
        histImg.at<float>(i-1,j) = makeHist(chrom1.col(j), chrom2.col(j));
	//histImg.at<float>(i-1,j) = makeHist(frame1.col(j), frame2.col(j));
	//cout << histImg.at<float>(i-1,j) << ", ";
      }
      frame1 = frame2;
    }
   
    
    for(int i = 0; i < histImg.rows; ++i) {
      for(int j = 0; j < histImg.cols; ++j) {
	cout << histImg.at<float>(i,j) << ", ";
      }
    }
    
    
    imshow("test", histImg);
    waitKey(0);
  }
  else cout << "Video not found" << endl;
}
