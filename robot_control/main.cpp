#include "example-client.h"
#include <stdio.h> 

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
