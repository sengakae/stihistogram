#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "math.h"

#include <iostream>
#include <string>

using namespace cv;
using namespace std;

// reads 2 images, extracts the red and green channels, create and compares histograms
float makeHist(Mat img1, Mat img2) {
  // error if row count does not match
  if(img1.rows != img2.rows) {
    cout << "Error - Row count does not match" << endl;
    exit(1);
  }
    
  // modified version of Sturges's Rule
  int bins = 1 + log(img2.rows)/log(2);

  // histogram setup
  float range[] = {0, 256};
  const float* histRange = {range};

  // split images into color channels
  vector<Mat> bgr_planes1;
  split(img1, bgr_planes1);

  vector<Mat> bgr_planes2;
  split(img2, bgr_planes2);

  // initialize histogram variables
  Mat rgHist1, rgHist2;

  // extract red and green channels from images
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

  // histogram calculation + normalization
  calcHist(&rg1, imgCount, channels, Mat(), rgHist1, dims, &bins, &histRange, true, false);
  normalize(rgHist1, rgHist1, 0, 1, NORM_MINMAX, -1, Mat());

  calcHist(&rg2, imgCount, channels, Mat(), rgHist2, dims, &bins, &histRange, true, false);
  normalize(rgHist2, rgHist2, 0, 1, NORM_MINMAX, -1, Mat());
 
  // each histogram is divided by its sum
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
  
  // return the histogram intersection
  return compareHist(rgHist1, rgHist2, CV_COMP_INTERSECT);
}

// chromaticity conversion
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

      // stores the pixel values into the new Mat output 
      res.data[res.step[0]*j + res.step[1]*i + 0] = (b*255);
      res.data[res.step[0]*j + res.step[1]*i + 1] = (g*255);
      res.data[res.step[0]*j + res.step[1]*i + 2] = (r*255);
    }
  }
  return res;
}

int main(int argc, char** argv) {
  string argv2(argv[2]);
  // reads video file as argument
  if(argc < 3 || ((argv2 != "cols") && (argv2 != "rows"))) {
    cout << "Usage: ./sti <video> <rows|cols>" << endl;
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
    
    // reads in first frame
    capture.set(CV_CAP_PROP_POS_FRAMES, 0);
    capture >> frame1;
    
    // for frames 2 and onwards
    for(int i = 1; i < vidframes - 1; ++i) {
      capture.set(CV_CAP_PROP_POS_FRAMES, i);
      capture >> frame2;
      if(frame1.cols == 0 || frame2.cols == 0) {
	cout << "frame: " << i << endl;
	return -1;
      }
      
      // convert frames before histogram creation
      // frames are resized to 128 x 128
      resize(frame1, frame1, Size(128, 128));
      Mat chrom1 = chromConv(frame1);
      
      resize(frame2, frame2, Size(128, 128));
      Mat chrom2 = chromConv(frame2);
      
      if(argv2 == "cols") {
	for(int j = 0; j < frame2.cols; ++j) {
	  histImg.at<float>(i-1,j) = makeHist(chrom1.col(j), chrom2.col(j));
	}
      }
      else if(argv2 == "rows") {
	for(int j = 0; j < frame2.rows; ++j) {
	  Mat row1, row2;
	  transpose(chrom1.row(j), row1);
	  transpose(chrom2.row(j), row2);
	  histImg.at<float>(i-1,j) = makeHist(row1, row2);
	}
      }
      frame1 = frame2;
    }    
    
    // transpose result 90 degrees
    // output resulting sti - press any key to exit
    transpose(histImg, histImg);
    imshow("STI", histImg);
    waitKey(0);
  }
}
