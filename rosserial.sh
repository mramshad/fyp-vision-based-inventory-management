
#! /bin/sh

cd $HOME
#xterm -hold -e "roscore" &
roscore &
sleep 2
source ~/catkin-ws/devel/setup.bash &&
#xterm -hold -e "rosrun rosserial_python serial_node.py _port:=/dev/ttyUSB0" 
roslaunch rosbridge_server rosbridge_websocket.launch &
sleep 2
rosrun mjpeg_server mjpeg_server _port:=8181 &
sleep 2
rosrun rosserial_python serial_node.py _port:=/dev/ttyUSB0 &
sleep 10
#roslaunch usb_cam-test.launch &
#sleep 2
source ~/catkin-ws/devel/setup.bash  &&
rosrun first_pkg panda 
exit 0


