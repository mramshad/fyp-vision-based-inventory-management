#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>
#include <std_msgs/Int8.h>
#include<std_msgs/String.h>

#include <opencv2/core/version.hpp>
#include <cstdlib>
#include <opencv2/highgui/highgui.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdlib.h>

using namespace std;
using namespace cv;

int middle_line=320;
Mat frame;
float b_dist;

struct locations{

    int x_low;
    int x_high;
    int y_low;
    int y_high;

}; 

//Publisher for blob distance from centre
ros::Publisher dist_pub, pub_user_cmnds, pub_feedback;

int counter=0;

////////Get Ditance ///////////////////////////////////////////////
float get_distance(vector< vector<Point> >contours){

    float dist;
    float temp_dist=0;
    float distance;
    bool detection_check=false;
    locations temp_location;
    for(int i=0;i<contours.size();i++){
        locations loc;
        loc.x_low=640;
        loc.x_high=0;
        loc.y_low=640;
        loc.y_high=0;
        vector <Point> points;
        points= contours.at(i);
        vector<Point>::iterator itr = points.begin();
        
        for(itr=points.begin();itr<points.end();itr++){
            Point temp=*itr;
            
            if(loc.x_high<temp.x){
                loc.x_high=temp.x;
            }
            if(loc.x_low>temp.x){
            
                loc.x_low=temp.x;
            }
            if(loc.y_high<temp.y){
            
                loc.y_high=temp.y;
            
            }
            if(loc.y_low>temp.y){
            
                loc.y_low=temp.y;
            
            }
        
        }
        
        dist=(loc.x_high+loc.x_low)/2;
        //cout <<dist << endl;
        if(dist<=middle_line){
            detection_check=true;
            if(dist>temp_dist){
                temp_location=loc;
                temp_dist=dist;
                distance=320-dist;
                
            }
        
        }
        
        
    }
    
    Point pt1,pt2;
        pt1.x=temp_location.x_low;
        pt1.y=temp_location.y_low;
        pt2.x=temp_location.x_high;
        pt2.y=temp_location.y_high;
        //Scalar colour(255,255,255);
        //rectangle(frame,pt1,pt2,colour,1,8,0);
        //imshow("name",frame);
        
        
        if(detection_check){
    return distance;
        }else{
        
            return middle_line;
            
        }
}
/////////////////////////////////////////////////////


//Malaka - Segmentation /////////////////////////////////////////////////////////////////////////////////////////////////////
float segment(Mat image){
   
float distance;    
//Mat image = imread("24.jpg");
//Mat image(image1.size(),image1.type());
Mat image1= image.clone();
cvtColor(image,image,CV_BGR2GRAY);
Mat bin(image.size(),image.type());
threshold(image,bin,150,255,CV_THRESH_BINARY);

Mat element = getStructuringElement( 0, Size(20,20 ), Point( 2,2 ) );
morphologyEx(bin,bin,MORPH_CLOSE,element);
morphologyEx(bin,bin,MORPH_ERODE,element);
element = getStructuringElement( 2, Size( 10,10 ), Point( 2,2 ) );
morphologyEx(bin,bin,MORPH_OPEN,element);
morphologyEx(bin,bin,MORPH_DILATE,element);

Mat inv = Scalar::all(255) - bin;
Mat res(image.size(),image.type());
image.copyTo(res,bin);
//namedWindow("Output");
//imshow("Output",bin);

vector < vector<Point> > contours;
vector<Vec4i> hyrachey;

findContours(bin,contours,hyrachey,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE,Point(0,0));
Mat cot = Mat::zeros(bin.rows,bin.cols,bin.type());
vector<locations> located;
distance=get_distance(contours);

//cout << "distance = "<< distance <<endl;
return distance;
}
//////////////////////////////////////





//Callback funcion to receive the images
void imageCallback(const sensor_msgs::ImageConstPtr& msg)
{
  //Malaka's code to blob detection
  b_dist=segment(cv_bridge::toCvShare(msg, "rgb8")->image);
  
  if(b_dist <15 && counter>100){
      std_msgs::String im_pub_msg;
      im_pub_msg.data = "stop"; 
      pub_user_cmnds.publish(im_pub_msg);
  
      im_pub_msg.data = "pd";
      pub_feedback.publish(im_pub_msg);
      counter=0;
    //ROS_INFO("Distance:   Counter: ", b_dist, counter);
  }
  /*try
  {
    cv::imshow("view", cv_bridge::toCvShare(msg, "bgr8")->image);
    //cv::WaitKey(30);
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
  }*/

  std_msgs::Int8 blob_dist;
  blob_dist.data=0;//output of malakas code;
  dist_pub.publish(blob_dist);
  counter++;
  ROS_INFO("Distance: %f  Counter: %d", b_dist, counter);
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "image_listener");
  ros::NodeHandle nh;
  cv::namedWindow("view");
  cv::startWindowThread();
  image_transport::ImageTransport it(nh);
  image_transport::Subscriber sub = it.subscribe("camera/image", 1, imageCallback);

  dist_pub = nh.advertise<std_msgs::Int8>("blob_dist", 10 , true);
  pub_user_cmnds = nh.advertise<std_msgs::String>("user_cmnds", 10 , true);
  pub_feedback = nh.advertise<std_msgs::String>("feedback", 10 , true);

  ros::spin();
  cv::destroyWindow("view");
}
