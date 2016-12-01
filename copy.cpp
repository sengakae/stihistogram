#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "math.h"

#include <iostream>
#include <string>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
  string argv2(argv[2]);
  // reads video file as argument
  if(argc < 3 || ((argv2 != "cols") && (argv2 != "rows"))) {
    cout << "Usage: ./copy <video> <rows|cols>" << endl;
    return -1;
  }    
  
  cout << "Filename: " << argv[1] << endl;

  Mat frame;
  VideoCapture capture;
  if(capture.open(argv[1])) {
    double fps = capture.get(CV_CAP_PROP_FPS);
    int vidframes = capture.get(CV_CAP_PROP_FRAME_COUNT);
    cout << "fps: " << fps <<  endl;
    cout << "total frames: " << vidframes << endl;
    
    // get first frame and create sti Mat using frame size and type
    capture.set(CV_CAP_PROP_POS_FRAMES, 0);
    capture >> frame;

    Size size;
    int center;
    if(argv2 == "cols") {
      size = Size(vidframes, frame.rows);
      center = frame.cols/2;
    }
    if(argv2 == "rows") {
      size = Size(vidframes, frame.cols);
      center = frame.rows/2;
    }
    
    Mat sti(size, frame.type());
      
    // for all frames, get the center column or row and copy it to the sti
    for(int i = 0; i < vidframes - 1; ++i) {
      capture.set(CV_CAP_PROP_POS_FRAMES, i);
      capture >> frame;

      if(argv2 == "cols") {
	frame.col(center).copyTo(sti.col(i));
      }
      if(argv2 == "rows") {
	Mat row;
	transpose(frame.row(center), row);
	row.copyTo(sti.col(i));
      }
    }
    
    // output resulting sti - press any key to exit
    imshow("STI", sti);
    waitKey(0);
  }
}
