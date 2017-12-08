#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/core.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

int erosion_size = 1;

float GREEN_COIN_SIZE = 24.25;

void eraseNoise(Mat & in,Mat & out){
    Mat elem = getStructuringElement(0,Size( 2*erosion_size + 1, 2*erosion_size+1 ),Point( erosion_size, erosion_size ));
	dilate (in, out, elem);
	erode (out, out, elem);
    dilate (out,out, elem);
}

vector<Vec3f> findCoins(Mat & gray,float minRad,float maxRad,float min_dist){
    if(min_dist == 0){
        min_dist = gray.rows/4;
    }
    vector<Vec3f> coin;
    HoughCircles(gray,coin,CV_HOUGH_GRADIENT,2,gray.rows/4,100,100,minRad,maxRad);
    return coin;
}

Vec3f findGreenCoin(std::vector<Vec3f> v){
    Vec3f greenCoin = v[0];
    int i = 0,index = 0;
    for(Vec3f vec:v){
        if(vec[0] < greenCoin[0]){
            //cout<<vec<<endl;
            greenCoin = vec;
            index = i;
        }
        i++;
    }
    v.erase(v.begin()+index);
    //cout<<greenCoin<<endl;
    return greenCoin;

}

void drawCoins(vector<Vec3f> coin,Mat & toDraw,float ratio){
    //int l=coin.size();
    //cout<<"\n The number of coins is: "<<l<<"\n\n";
    // To draw the detected circles.
    for( size_t i = 0; i < coin.size();i++){
        Point center(cvRound(coin[i][0]),cvRound(coin[i][1]));
        // Detect center
        // cvRound: Rounds floating point number to nearest integer.
        int radius=cvRound(coin[i][2]);
        // To get the radius from the second argument of vector coin.
        circle(toDraw,center,3,Scalar(0,255,0),-1,8,0);
        // circle center
        //  To get the circle outline.
        circle(toDraw,center,radius,Scalar(0,0,255),3,8,0);
        if(ratio > 0){
            float mm = (coin[i][2]*2)/ratio;
            stringstream stream;
            stream<<fixed<<setprecision(2)<<mm;
            string mm_string = stream.str();
            putText(toDraw, mm_string, center,FONT_HERSHEY_SCRIPT_SIMPLEX,0.8, cvScalar(0,0,0),1, CV_AA);

        }
        // circle outline
        //cout<< " Center location for circle "<<i+1<<" :"<<center<<"\n Diameter : "<<2*radius<<endl;
    }
}

int main(int argc, char** argv){
    VideoCapture cap;
    // open the default camera, use something different from 0 otherwise;
    // Check VideoCapture documentation.
    if(!cap.open(0))
        return 0;

    int fps = cap.get(CV_CAP_PROP_FPS);
    int nb = 0;
    cout <<"Camera frame rate : "<<fps<<endl;
    vector<Vec3f> coin;
    Mat grey,frame;
    Vec3f greenCoin;
    float ratio = 0;
    float minRad = 0,maxRad = 0,min_dist = 0;
    for(;;){
        if(nb > fps)
            nb = 0;

        cap >> frame;
        if( frame.empty() ) break; // end of video stream

        if(nb%(fps/10) == 0){
            cvtColor(frame,grey,CV_BGR2GRAY);
            eraseNoise(grey,grey);
            coin = findCoins(grey,minRad,maxRad,min_dist);
            if(!coin.empty()){
                greenCoin = findGreenCoin(coin);
                ratio = (greenCoin[2]*2)/GREEN_COIN_SIZE;
                maxRad = greenCoin[2]+8*ratio;
                minRad = greenCoin[2]-11*ratio;
                min_dist = 16.25 * ratio;
            }
        }
        drawCoins(coin,frame,ratio);

        //cout<<endl;
        imshow("Coin counter", frame);
        if( waitKey(10) == 27 ) break; // stop capturing by pressing ESC
        nb++;
    }

    return 0;
}
