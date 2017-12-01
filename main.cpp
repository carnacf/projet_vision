/*#include "opencv2/opencv.hpp"

using namespace cv;

int main(int argc, char** argv){
    VideoCapture cap;
    // open the default camera, use something different from 0 otherwise;
    // Check VideoCapture documentation.
    if(!cap.open(0))
        return 0;
    for(;;){
          Mat frame;
          cap >> frame;
          if( frame.empty() ) break; // end of video stream
          imshow("this is you, smile! :)", frame);
          if( waitKey(10) == 27 ) break; // stop capturing by pressing ESC
    }
    // the camera will be closed automatically upon exit
    // cap.close();
    return 0;
}*/

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

int main(){
    Mat image;
    image=imread("IMG-20160425-WA0002.jpg",CV_LOAD_IMAGE_GRAYSCALE);

    vector<Vec3f> coin;

    HoughCircles(image,coin,CV_HOUGH_GRADIENT,2,20,450,60,0,0 );

    int l=coin.size();

    cout<<"\n The number of coins is: "<<l<<"\n\n";
    // To draw the detected circles.
    for( size_t i = 0; i < coin.size(); i++ ){
    	Point center(cvRound(coin[i][0]),cvRound(coin[i][1]));
    	// Detect center
    	// cvRound: Rounds floating point number to nearest integer.
    	int radius=cvRound(coin[i][2]);
    	// To get the radius from the second argument of vector coin.
    	circle(image,center,3,Scalar(0,255,0),-1,8,0);
        // circle center
        //  To get the circle outline.
    	circle(image,center,radius,Scalar(0,0,255),3,8,0);
        // circle outline
    	cout<< " Center location for circle "<<i+1<<" :"<<center<<"\n Diameter : "<<2*radius<<endl;
    }
    cout<<endl;

    namedWindow("Coin Counter",CV_WINDOW_AUTOSIZE);
    imshow("Coin Counter",image);
    // first argument: name of the window
    // second argument: image to be shown(Mat object)

    waitKey(0); // Wait for infinite time for a key press.

    return 0;	// Return from main function.
}
