#include<ros/ros.h>
#include<std_msgs/String.h>
#include <unistd.h>

ros::Publisher panda_pub_ard, panda_pub_usr;
ros::Subscriber panda_sub_ard, panda_sub_usr;
bool ard_ready=true;


void user_cmndsCb(const std_msgs::String::ConstPtr& msg){
  ROS_INFO("Receiving user command:  %s", msg->data.c_str());

  std_msgs::String command;
  command.data = msg->data;

  if(msg->data == "stop"){
    panda_pub_ard.publish(command);
    ard_ready=false;
    ROS_INFO("Publishing to arduino %s", msg->data.c_str());
  }
  else if(msg->data == "run" || msg->data == "go" || msg->data == "back" || msg->data == "left" || msg->data == "right" ){
    if(ard_ready){
      panda_pub_ard.publish(command);
      ard_ready=false;
      ROS_INFO("Publishing to arduino %s", msg->data.c_str());
    }
    else{
      command.data="robot is busy";
      panda_pub_usr.publish(command);
      ROS_INFO("Publishing to user %s", command.data.c_str());
    }
  }
  else{
    command.data="Invalid Command";
    panda_pub_usr.publish(command);
    ROS_INFO("Publishing to user %s", command.data.c_str());
  }

}

void feedbackCb(const std_msgs::String::ConstPtr& msg){
   ROS_INFO("Feedback received:  %s", msg->data.c_str());
   std_msgs::String response, command;
   response.data = msg->data;

   if(response.data == "td" || response.data == "od" || response.data == "ed" ||    
   response.data == "id" || response.data == "ad" || response.data == "md"){
     ard_ready=true;
     panda_pub_usr.publish(response);
     ROS_INFO("Publishing to user %s", response.data.c_str());
   }
   else if(response.data == "pd"){
     //command.data="run";
     //usleep(5000 * 1000);
     //panda_pub_ard.publish(command);
     ard_ready=true;
     panda_pub_usr.publish(response);
     ROS_INFO("Publishing to user %s", response.data.c_str());
   }
   else if(response.data == "ud"){
     command.data="hm";
     panda_pub_ard.publish(command);
     ard_ready=false;
     ROS_INFO("Publishing to arduino %s", command.data.c_str());
   }
   else{
     response.data="something went wrong :(";
     panda_pub_usr.publish(response);
     ROS_INFO("Publishing to user %s", response.data.c_str());
   }
}

int main(int argc, char** argv){
 
  ros::init(argc,argv, "panda");
  ROS_INFO("ros initializing");

  ros::NodeHandle nh;
 
  std_msgs::String msg;

  panda_pub_ard = nh.advertise<std_msgs::String>("toggle_led", 10 , true);
  panda_pub_usr = nh.advertise<std_msgs::String>("response", 10 , true);
  panda_sub_usr = nh.subscribe("user_cmnds", 1000, user_cmndsCb);
  panda_sub_ard = nh.subscribe("feedback", 1000, feedbackCb);
  
  ros::spin();
  return 0; 
}



