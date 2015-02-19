
#! /bin/sh

cd $HOME
#xterm -hold -e "roscore" &
roscore &
sleep 2
#xterm -hold -e "rosrun rosserial_python serial_node.py _port:=/dev/ttyUSB0" 
roslaunch rosbridge_server rosbridge_websocket.launch &
sleep 2
rosrun mjpeg_server mjpeg_server _port:=8181
sleep 2
rosrun rosserial_python serial_node.py _port:=/dev/ttyUSB3 &
sleep 10
source ~/catkin_ws/devel/setup.bash 
rosrun first_pkg panda
exit 0


