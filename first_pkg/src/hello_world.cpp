// Include the ROS C++ APIs
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <iostream>
#include "hello_world.h"
using namespace std;

// Standard C++ entry point



  /*void robot(int argc, char** argv){
    ros::init(argc, argv, "panda_pub");
    ROS_INFO("Initializing");

    ros::NodeHandle nh;
    ros::Publisher panda_pub;
    ros::Subscriber panda_sub;
    
    std_msgs::String msg;
    
    panda_pub =nh.advertise<std_msgs::String>("toggle_led",10, true); 
    msg.data="run";
    panda_pub.publish(msg);
    ROS_INFO("Publishing");
    ros::spin();
  }*/

  
  // panda_pub =nh.advertise<std_msgs::String>("toggle_led",10, true); 

  // panda_sub = nh.subscribe("feedback", 1000, feedbackCb);

  /*void run(){
    panda_pub =nh.advertise<std_msgs::String>("toggle_led",10, true); 
    msg.data="run";
    panda_pub.publish(msg);
    ROS_INFO("Publishing");
  }*/




void feedbackCb(const std_msgs::String::ConstPtr& msg){
    ROS_INFO("Receiving:  %s", msg->data.c_str());
}

int main(int argc, char** argv) {
  // Announce this program to the ROS master as a "node" called "hello_world_node"

   
  
  ros::init(argc, argv, "panda_pub");
  //robot(argc, argv);
  ros::NodeHandle nh;
  ros::Publisher panda_pub;
  ros::Subscriber panda_sub;
  
  panda_pub =nh.advertise<std_msgs::String>("user_cmnds",10, true);
  panda_sub = nh.subscribe("feedback", 1000, feedbackCb);

  std_msgs::String msg;

  msg.data="run";
  panda_pub.publish(msg);
  ROS_INFO("Publishing");
  //run();

  ros::spin();
  //ros::shutdown();
  return 0;
}
