#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/core.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

int erosion_size = 1;
float LEFT_COIN_SIZE = 24.25;

typedef struct Coin{
    Point center;
    float rad;
}Coin;

void eraseNoise(Mat & in,Mat & out){
    Mat elem = getStructuringElement(0,Size( 2*erosion_size, 2*erosion_size),Point(erosion_size,erosion_size));
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

Vec3f findLeftCoin(std::vector<Vec3f> v){
    Vec3f greenCoin = v[0];
    int i = 0,index = 0;
    for(Vec3f vec:v){
        if(vec[0] < greenCoin[0]){
            greenCoin = vec;
            index = i;
        }
        i++;
    }
    v.erase(v.begin()+index);
    return greenCoin;

}

void drawCoins(vector<Vec3f> coin,Mat & toDraw,float ratio){
    for( size_t i = 0; i < coin.size();i++){
        Point center(cvRound(coin[i][0]),cvRound(coin[i][1]));
        int radius=cvRound(coin[i][2]);
        circle(toDraw,center,3,Scalar(0,255,0),-1,8,0);
        circle(toDraw,center,radius,Scalar(0,0,255),3,8,0);
        if(ratio > 0){
            float mm = (coin[i][2]*2)/ratio;
            stringstream stream;
            stream<<fixed<<setprecision(2)<<mm;
            string mm_string = stream.str();
            putText(toDraw, mm_string, center,FONT_HERSHEY_SCRIPT_SIMPLEX,0.8, cvScalar(0,0,0),1, CV_AA);
        }
    }
}

void meanCoins(vector<Coin> & mean,vector<Vec3f> & toAdd,float max_dist){
    if(mean.empty()){
        for(Vec3f v:toAdd){
            Coin c;
            c.center = Point(v[0],v[1]);
            c.rad = v[2];
        }
    }else{
        vector<Vec3f> toad = toAdd;
        for(size_t j  = 0;j<mean.size();j++){
            int index = -1;
            float min_dist = numeric_limits<float>::max();
            for(size_t i = 0;i<toad.size();i++){
                Point difference = mean[j].center-Point(toad[i][0],toad[i][1]);
                double distance = sqrt( difference.ddot(difference));
                if(distance < min_dist && distance < max_dist){
                    index = i;
                    min_dist = distance;
                }
            }
            mean[j].center = (mean[j].center + Point(toad[index][0],toad[index][1]))/2;
            mean[j].rad = (mean[j].rad + toad[index][2])/2;
            mean.erase(mean.begin()+index);
        }
        if(!toad.empty()){
            for(Vec3f v:toAdd){
                Coin c;
                c.center = Point(v[0],v[1]);
                c.rad = v[2];
            }
        }
    }
}

int main(int argc, char** argv){
    VideoCapture cap;
    if(!cap.open(0))
        return 0;

    int fps = cap.get(CV_CAP_PROP_FPS);
    int nb = 0;
    vector<Coin> mean;
    vector<Vec3f> coin;
    Mat grey,frame;
    Vec3f leftCoin;
    float ratio = 0;
    float minRad = 0,maxRad = 0,min_dist = 0;



    cout <<"Camera frame rate : "<<fps<<endl;

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
                leftCoin = findLeftCoin(coin);
                ratio = (leftCoin[2]*2)/LEFT_COIN_SIZE;
                maxRad = leftCoin[2]+8*ratio;
                minRad = leftCoin[2]-11*ratio;
                min_dist = 16.25 * ratio;
            }
        }
        meanCoins(mean,coin,min_dist);
        drawCoins(coin,frame,ratio);

        //cout<<endl;
        imshow("Coin counter", frame);
        if( waitKey(10) == 27 ) break; // stop capturing by pressing ESC
        nb++;
    }

    return 0;
}
