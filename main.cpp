#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/core.hpp>
#include <iostream>
#include <stdio.h>

float LEFT_COIN_SIZE = 22.25;

int erosion_size = 1;

using namespace std;
using namespace cv;

typedef struct Coin{
    Point center;
    float rad;
}Coin;

double argent[8][2] = {
    {2,25.75},
    {1,23.25},
    {0.5,24.25},
    {0.2,22.25},
    {0.1,21.25},
    {0.05,18.75},
    {0.01,16.25}
};

float closestCoinTo(Mat & color,Vec3f coin, float ratio)
{
    std::cerr << "strat" << '\n';
    float diam = (coin[2]*2)/ratio;
    float dif = numeric_limits<float>::max();
    float temp_dif = 0;
    int ind = 0;
    int start = 0;
    if(color.at<Vec3b>(coin[0],coin[1]).val[1] > 180)
    {
        start = 5;
    }

    for(int i = start;i<7;i++)
    {
        temp_dif = argent[i][1] - diam;
        if(temp_dif < 0) temp_dif = 0;
        if(temp_dif < dif)
        {
            dif = temp_dif;
            ind = i;
        }
    }
    std::cerr << "end" << '\n';
    return argent[ind][0];
}

vector<Vec3f> findCoins(Mat & gray,float minRad,float maxRad,float min_dist){
    if(min_dist == 0){
        min_dist = gray.rows/4;
    }
    vector<Vec3f> coin;
    HoughCircles(gray,coin,CV_HOUGH_GRADIENT,2,gray.rows/4,50,100,minRad,maxRad);
    return coin;
}

Vec3f findLeftCoin(std::vector<Vec3f> & v){
    Vec3f greenCoin = v[0];
    int i = 0,index = 0;
    for(Vec3f vec:v){
        if(vec[0] < greenCoin[0]){
            greenCoin = vec;
            index = i;
        }
        i++;
    }
    greenCoin = v[index];
    return greenCoin;
}

void drawCoins(vector<Vec3f> coin,Mat & toDraw,float ratio){
    std::vector<float> v;
    for( size_t i = 0; i < coin.size();i++){
        Point center(cvRound(coin[i][0]),cvRound(coin[i][1]));
        int radius=cvRound(coin[i][2]);
        circle(toDraw,center,3,Scalar(0,255,0),-1,8,0);
        circle(toDraw,center,radius,Scalar(0,0,255),3,8,0);
        if(ratio > 0){
            float mm = closestCoinTo(toDraw,coin[i],ratio);
            v.push_back(mm);
            stringstream stream;
            stream<<fixed<<setprecision(2)<<mm;
            string mm_string = stream.str();
            putText(toDraw, mm_string, center,FONT_HERSHEY_SCRIPT_SIMPLEX,0.8, cvScalar(0,0,0),1, CV_AA);
        }
    }
    float s = 0;
    for(float b:v){
        s+=b;
    }
    stringstream stream;
    stream<<fixed<<setprecision(2)<<s;
    string mm_string = stream.str();
    putText(toDraw,mm_string + " €",Point(10,50),FONT_HERSHEY_SCRIPT_SIMPLEX,0.8, cvScalar(0,0,0),1, CV_AA);
}

void meanCoins(vector<Coin> & mean,vector<Vec3f> & toAdd,float max_dist){
    if(mean.empty()){
        for(Vec3f v:toAdd){
            Coin c;
            c.center = Point(v[0],v[1]);
            c.rad = v[2];
            mean.push_back(c);
        }
    }else if(!toAdd.empty()){

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
            //mean.erase(mean.begin()+index);
        }
        if(!toad.empty()){
            for(Vec3f v:toad){
                Coin c;
                c.center = Point(v[0],v[1]);
                c.rad = v[2];
                mean.push_back(c);
            }
        }
    }
}

void eraseNoise(Mat & in,Mat & out){
    Mat elem = getStructuringElement(1,Size( 2*erosion_size+1, 2*erosion_size+1),Point(erosion_size,erosion_size));
    dilate (in, out, elem);
    erode (out, out, elem);
}

int main(int argc, char** argv){
    VideoCapture cap;
    if(!cap.open(1))
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

    while(true){
        if(nb > fps)
            nb = 0;

        cap >> frame;
        if( frame.empty() ) break; // end of video stream

        if(nb%(fps/10) == 0){
            cvtColor(frame,grey,CV_BGR2GRAY);
            equalizeHist(grey,grey);
            eraseNoise(grey,grey);
            GaussianBlur(grey,grey,Size(11,11),0);
            medianBlur(grey,grey,7);



            //medianBlur(grey,grey,7);
            //GaussianBlur(grey,grey,Size(11,11),0);


            imshow("file",grey);
            Mat canny;
            Canny(grey,canny,50,100);
            imshow("hist",canny);


            coin = findCoins(grey,minRad,maxRad,min_dist);
            if(!coin.empty()){
                leftCoin = findLeftCoin(coin);
                ratio = (leftCoin[2]*2)/LEFT_COIN_SIZE;
                maxRad = leftCoin[2]+8*ratio;
                minRad = leftCoin[2]-11*ratio;
                min_dist = (16.25 * ratio)/2;
                meanCoins(mean,coin,min_dist);
            }

        }
        drawCoins(coin,frame,ratio);


        //cout<<endl;
        //imshow("Coin counter", grey);
        imshow("Coin counter colored", frame);
        if( waitKey(10) == 27 ) break; // stop capturing by pressing ESC
        nb++;
    }

    return 0;
}
