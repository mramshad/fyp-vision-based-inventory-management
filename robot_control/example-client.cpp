//#include "easywsclient.hpp"
#include "json.h"
#include "easywsclient.cpp" // <-- include only if you don't want compile separately
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string>
#include <pthread.h>
#include <iostream>


using easywsclient::WebSocket;
static WebSocket::pointer ws = NULL;


std::string response;
bool response_received=true;


//Open a websocket and create publisher on topic user_cmnds

void publish(std::string command){
 response_received=false;

 json::Object msg;
 msg["data"]=command;

 json::Object pubObj;
 pubObj["op"]="publish";
 pubObj["topic"]="user_cmnds";
 pubObj["msg"]= msg;

 ws->send(json::Serialize(pubObj));

 json::Object unadvObj;
 unadvObj["op"]="unadvertise";
 unadvObj["topic"]="feedback";
 unadvObj["type"]= "std_msgs/String";

 //ws->send(json::Serialize(unadvObj));
}

void handle_message(const std::string & message)
{
 json::Object my_object = json::Deserialize(message);
 
 json::Object::ValueMap::iterator it = my_object.find("msg");
 json::Object::ValueMap::iterator it1;

 if (it != my_object.end())
 {
   json::Object msgObj = it->second;
   it1 = msgObj.find("data");
 }
 response=it1->second.ToString();
 if(response=="pd"){
   //retrive image
   printf("Retrive Image\n");
   publish("run");
 }
 else if(response=="md" || response=="td" || response=="ed" || response=="id"){
  response_received=true;
  //ws->close();
 }
 //printf("value: %s\n",  it1->second.ToString().c_str());
 //printf(">>> %s\n", message.c_str());
 //if (message == "world") { ws->close(); }
}

//Subscriber to response topic
void *listen(void *threadid){

 printf("Creating listener thread\n");
 //openWS();  

 json::Object subObj;
 subObj["op"]="subscribe";
 subObj["topic"]="response";
 subObj["type"]= "std_msgs/String";
 
 ws->send(json::Serialize(subObj));

 while (ws->getReadyState() != WebSocket::CLOSED ) {
  ws->poll();
  ws->dispatch(handle_message);
 }

 json::Object obj3;
 obj3["op"]="unsubscribe";
 obj3["topic"]="response";
 obj3["type"]= "std_msgs/String";
 
 pthread_exit(NULL);
 //ws->send(json::Serialize(obj3));
}

void openWS(){
 #ifdef _WIN32
 INT rc;
 WSADATA wsaData;
 rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
 if (rc) {
  printf("WSAStartup Failed.\n");
  return 1;
 }
 #endif
 ws = WebSocket::from_url("ws://localhost:9090");
 assert(ws);

 json::Object advObj;
 advObj["op"]="advertise";
 advObj["topic"]="user_cmnds";
 advObj["type"]= "std_msgs/String";

 ws->send(json::Serialize(advObj));

 pthread_t thread;
 int rc;
 int i=0;
 rc = pthread_create(&thread, NULL, listen, &i);
 /*if (rc){
         cout << "Error:unable to create thread," << rc << endl;
         exit(-1);
 }*/
}

void deleteWS(){
 delete ws;
 #ifdef _WIN32
 WSACleanup();
 #endif 
}




int main()
{
 openWS();  
 
 if(response_received){
 publish("run");}
 else {printf("Busy\n");}
 
 if(response_received){
 publish("stop");}
 else {printf("Busy\n");}
 
 if(response_received){
 publish("right");}
 else {printf("Busy\n");}
 
 //deleteWS();
 pthread_exit(NULL);
 return 0;
}
