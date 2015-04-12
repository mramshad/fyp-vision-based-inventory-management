/* ROS Serial Subscriber */
#include <ros.h>
#include <std_msgs/String.h>
#include <QTRSensors.h>
#include <Wire.h>
#include <Servo.h> 
#include <std_msgs/Int8.h>
#define NUM_SENSORS             6  // number of sensors used
#define NUM_SAMPLES_PER_SENSOR  4  // average 4 analog samples per sensor reading
//#define EMITTER_PIN             2  // emitter is controlled by digital pin 2

// sensors 0 through 5 are connected to analog inputs 0 through 5, respectively
QTRSensorsAnalog qtra((unsigned char[]) {0, 1, 2, 3, 4, 5},NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, QTR_NO_EMITTER_PIN);

Servo myservo;  // create servo object to control a servo 


// Define variable for sensor readings
unsigned int sensors[6]; // Matriz para armazenar valores dos sensores

//define the pins for pwm control
int Right = 5; 
int RightD= 4;
int Left = 3;
int LeftD = 2;
int pos = 90; //default postion of pan servo

unsigned int position;
int last_proportional;  // last propprtonal error
int integral;           // integration of previous errors
int proportional;       // present error
int derivative;         // error rate
int power_difference;   // required power difference for error correction
int blob_dist;
const int max = 80;     // maximum pwm speed of motors


char state,curr_state, pan;       //variables that decode user commands
unsigned int counter;             //variable used to run loops (free-runs)
unsigned int once=0;              // parameter used for one time calibration purpose
unsigned int stripes=0;           //variable to count the number of stripes
//unsigned int pass;
boolean sent=true;     //variable to confirm that feedback is sent to main control board(pandaboard)
char *fb;

unsigned long time;    //variable to measure processing times
char res[50];          //character buffer to convert int and string to constant char *

//ros node creation
ros::NodeHandle nh;

//Callback function for motor control commands
void messageCb (const std_msgs::String& toggle_msg){
  state = toggle_msg.data[1];  
}

//Callback function for pan-servo control commands
void messageCbPan(const std_msgs::String& pan_msg){  
  pan = pan_msg.data[1];
}

//Callback function for topic blob_dist to receive blob distance
void messageCbBlob(const std_msgs::Int8& blob_msg){  
  blob_dist = blob_msg.data;
}

//ros subscribers for usercommands for motor control and pan control
ros::Subscriber<std_msgs::String> sub("toggle_led", &messageCb);
ros::Subscriber<std_msgs::String> subPan("pan", &messageCbPan);
ros::Subscriber<std_msgs::Int8> subBlob("blob_dist", &messageCbBlob);

//Strings to send feedback msgs and logs to the control board(pandaboard)
std_msgs::String feedback,logs;

//ros Publishers to send feedback and log info
ros::Publisher pub("feedback",&feedback);
ros::Publisher pub_logs("logs",&logs);


// Acionamento dos motores
void set_motors(int left_speed, int right_speed){
  if(right_speed >= 0 && left_speed >= 0){
    digitalWrite(RightD, LOW);
    analogWrite(Right, right_speed);
    digitalWrite(LeftD, LOW);
    analogWrite(Left, left_speed);
  }
  if(right_speed >= 0 && left_speed < 0){
    left_speed = -left_speed;
   digitalWrite(RightD, LOW );
    analogWrite(Right, right_speed);
    digitalWrite(LeftD, HIGH);
    analogWrite(Left, left_speed);
  }
  if(right_speed < 0 && left_speed >= 0){
    right_speed = -right_speed;
    digitalWrite(RightD, HIGH);
    analogWrite(Right, right_speed);
    digitalWrite(LeftD, LOW);
    analogWrite(Left, left_speed);
  } 
  if(right_speed < 0 && left_speed < 0){
    right_speed = -right_speed;
    left_speed = -left_speed;
    digitalWrite(RightD, HIGH);
    analogWrite(Right, right_speed);
    digitalWrite(LeftD, HIGH);
    analogWrite(Left, left_speed);
  } 
}

//turn the robot to 90 degrees right
void turn_right_90(){
  int i;
  for(i=0;i<30000;i++){
    set_motors(155, -155);
  }
}

//turn the robot to 90 degrees left
void turn_left_90(){
  int i;
  for(i=0;i<30000;i++){
    set_motors(-155, 155);
  }
}

//turn robot by 180 degrees
void turn_180(){
  for(int i=0;i<30000;i++){
    set_motors(155, -155);
  }
  for(int j=0;j<30000;j++){
    set_motors(155, -155);
  }
}

//line following 
void follow_line(){
    position = qtra.readLine(sensors,QTR_EMITTERS_ON,true);
    
    //Serial.begin(9600);
     // 1000 means minimum reflectance, followed by the line position
    /*for (unsigned char i = 0; i < NUM_SENSORS; i++)
    {
      Serial.print(sensors[i]);
      Serial.print('\t');
    }
    Serial.println();
    Serial.println(position);
    delay(1000); */
    // O termo proporcional deve ser 0 quando estamos na linha
    proportional = ((int)position) - 2500;
    
    // Calcula o termo derivativo (mudança) e o termo integral (soma)
    // da posição
    derivative = proportional - last_proportional;
    integral += proportional;
    
    // Lembrando a ultima posição
    last_proportional = proportional;
    
    // Calcula a diferença entre o aranjo de potência dos dois motores
    // m1 - m2. Se for um número positivo, o robot irá virar para a 
    // direita. Se for um número negativo, o robot irá virar para a esquerda
    // e a magnetude dos números determinam a agudez com que fará as curvas/giros
    if(position>3500 || position<1500){
     power_difference = proportional/100+ integral/10000 + derivative*3/2;}
    else{
       power_difference = proportional/100+ integral/10000 ;
    }
    // Calcula a configuração atual dos motores.  Nunca vamos configurar
    // um motor com valor negativo
    if(power_difference > max)
      power_difference = max;
    if(power_difference < -max)
      power_difference = -max;
    
    //else{
      if(power_difference < 0)
        set_motors(max, max+power_difference);
      else
        set_motors(max-power_difference, max);
    //}
}

void setup(){
  Serial.begin(9600);
 
  nh.initNode();
  nh.subscribe(sub);
  nh.subscribe(subPan);
  nh.subscribe(subBlob);
  nh.advertise(pub);
  nh.advertise(pub_logs);
  
  pinMode(4,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(13,OUTPUT);
  
  myservo.attach(11);
  myservo.write(pos);
  
}

void loop(){
   //time=millis();
   
   if(state=='u')
   { 
     curr_state=state;
     sent=false;
     if(once<1){
       set_motors(0,0); // Enquanto espera, motores permanecem parados
 
     for(counter=0; counter<80; counter++){
       if(counter < 20 || counter >= 60){
         set_motors(50,-50); // Gira para a direita
       }
       else{
         set_motors(-50,50); // Gira para a esquerda
       }
       // Esta função armazena um conjunto de leituras dos sensores, e mantém
       // informações sobre o máximo e mínimo valores encontrados
       qtra.calibrate();
       // Desde que contamos até 80, o total do tempo de calibração
       // será de 80 * 10 = 800 ms
       delay(10);
     }
     set_motors(0,0); // Garante motores parados após o processo 
                   // de calibração
     once++;
   }
   
   //skip the first stripe ie. starting position  
   if(stripes==0){
     follow_line(); 
     if(sensors[0]<100 && sensors[5]<100){
       stripes++;
       for(int i=0;i< 10000;i++){
         set_motors(50,50);
       }
     }  
   }
   
   else{
     follow_line();
    
     if(sensors[0]<100 && sensors[5]<100){
       fb="pd";
       feedback.data=fb;
       /*for(int i=0; i< 30000;i++){
         set_motors(0,0);
       }*/
       stripes++;
      
       for(int i=0;i< 10000;i++){
         set_motors(50,50);
       }
       pub.publish(&feedback);
       sent=true;
       state='t';
     } 
     
     else if(sensors[2]>700 && sensors[3]>700){
       feedback.data="ud";
       turn_180();
       pub.publish(&feedback);
       sent=true;
       state='t';
     }
     /*else if(sensors[4]<100 && sensors[5]<100){
       if(sensors[2]<100 && sensors[3]<100){
         if(sensors[0]>700){
           for(int i=0; i< 20000;i++){
             set_motors(50,50);
           }
           turn_left_90();
           state='t';
         }
       }
     }
     else if(sensors[0]<100 && sensors[1]<100){
       if(sensors[2]<100 && sensors[3]<100){
         if(sensors[5]>700){
           for(int i=0; i< 20000;i++){
             set_motors(50,50);
           }
           turn_right_90();
           state='t';
         }
       }
     }*/   
     
   }
//   sprintf(res,"state:%c time:%lu s1:%d s2:%d s3:%d s4:%d s5:%d s6:%d", curr_state, time, sensors[0],sensors[1],sensors[2],sensors[3],sensors[4],sensors[5]);
//   logs.data=res;
//   pub_logs.publish(&logs);
  }
  
  else if(state=='m'){
    curr_state=state;
    sent=false;
    fb="md";
    feedback.data=fb;
    follow_line();
    if(sensors[2]>500 && sensors[3]>500){
      for(int i=0; i<25000;i++){
        set_motors(70,70);
      }
      for(int i=0; i<25000;i++){
        set_motors(50,50);
      }
      for(int i=0; i<25000;i++){
        set_motors(30,30);
      }
      for(int i=0; i<5000;i++){
        set_motors(0,0);
      }
      turn_180();
      pub.publish(&feedback);
      sent=true; 
      state='t';
    }
//    sprintf(res,"state:%c time:%lu s1:%d s2:%d s3:%d s4:%d s5:%d s6:%d", curr_state, time, sensors[0],sensors[1],sensors[2],sensors[3],sensors[4],sensors[5]);
//    logs.data=res;
//    pub_logs.publish(&logs);
  } 
  
  else if(state=='i') 
  { 
    curr_state=state;
    sent=false;
    //fb="id";
    //feedback.data=fb;
    for(counter=0;counter<50;counter++){
      set_motors(30,-30);
      delay(10);
    }
    //set_motors(0,0);
    //pub.publish(&feedback);
    //sent=true;
    state='t';
//    sprintf(res,"state:%c time:%lu s1:%d s2:%d s3:%d s4:%d s5:%d s6:%d", curr_state, time, sensors[0],sensors[1],sensors[2],sensors[3],sensors[4],sensors[5]);
//    logs.data=res;
//    pub_logs.publish(&logs);
  }
  
  else if(state=='e') 
  {
    curr_state=state;
    sent=false;
    //feedback.data="left_done"; 
    for(counter=0;counter<50;counter++){
      set_motors(-30,30);
      delay(10);
    }
    state='t';
//    sprintf(res,"state:%c time:%lu s1:%d s2:%d s3:%d s4:%d s5:%d s6:%d", curr_state, time, sensors[0],sensors[1],sensors[2],sensors[3],sensors[4],sensors[5]);
//    logs.data=res;
//    pub_logs.publish(&logs);
  }
  
  else if(state=='o')
  {
    curr_state=state;
    sent=false;
    set_motors(80,80);
    /*digitalWrite(13,HIGH);
    delay(500);
    digitalWrite(13,LOW);
    delay(500);*/
//    sprintf(res,"state:%c time:%lu s1:%d s2:%d s3:%d s4:%d s5:%d s6:%d", curr_state, time, sensors[0],sensors[1],sensors[2],sensors[3],sensors[4],sensors[5]);
//    logs.data=res;
//    pub_logs.publish(&logs);
  }
  
  else if(state=='a')
  {
    curr_state=state;
    sent=false;
    set_motors(-50,-50);
//    sprintf(res,"state:%c time:%lu s1:%d s2:%d s3:%d s4:%d s5:%d s6:%d", curr_state, time, sensors[0],sensors[1],sensors[2],sensors[3],sensors[4],sensors[5]);
//    logs.data=res;
//    pub_logs.publish(&logs);
  }
  else if(state=='t'){
    curr_state=state;
    sent=false;
    if(!sent){
      fb="td";
      feedback.data=fb;
      pub.publish(&feedback);
      sent=true; 
      state='z';
    }
    set_motors(0,0);
    //pass=0;
//    sprintf(res,"state:%c time:%lu s1:%d s2:%d s3:%d s4:%d s5:%d s6:%d", curr_state, time, sensors[0],sensors[1],sensors[2],sensors[3],sensors[4],sensors[5]);
//    logs.data=res;
//    pub_logs.publish(&logs);
  }
  
  if (pan=='l'){
    if(pos<180){
      pos = pos + 10;
    }
    else {
      pos = 180;
    }
    myservo.write(pos);
    pan='t';
  }
  else if(pan=='r'){
    if(pos>0){
      pos = pos -10;
    }
    else {
      pos = 0;
    }
    myservo.write(pos);
    pan='t';
  }
  
  nh.spinOnce();
  delay(1);
}



