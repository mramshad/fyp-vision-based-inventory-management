/* ROS Serial Subscriber */
#include <ros.h>
#include <std_msgs/String.h>
#include <QTRSensors.h>
#include <Wire.h>
#include <Servo.h> 

#define NUMREADINGS 10 

#define NUM_SENSORS             6  // number of sensors used
#define NUM_SAMPLES_PER_SENSOR  4  // average 4 analog samples per sensor reading
//#define EMITTER_PIN             2  // emitter is controlled by digital pin 2

// sensors 0 through 5 are connected to analog inputs 0 through 5, respectively
QTRSensorsAnalog qtra((unsigned char[]) {0, 1, 2, 3, 4, 5}, 
  NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, QTR_NO_EMITTER_PIN);

Servo myservo;  // create servo object to control a servo 


// Definição de variáveis
unsigned int sensors[6]; // Matriz para armazenar valores dos sensores

int Right = 5; // Pinagem para a PONTE-H
int RightD= 4;
int Left = 3;
int LeftD = 2;
int pos = 90;

unsigned int position;
int last_proportional;
int integral;
int proportional;
int derivative;
int power_difference;
const int max = 60;

char state, pan;
unsigned int counter; // usado como um simples contador
unsigned int once=0; 
unsigned int stripes=0;
unsigned int pass;
boolean sent=true;
char *fb;


ros::NodeHandle nh;

void messageCb (const std_msgs::String& toggle_msg){
  
  state = toggle_msg.data[1];
  /*digitalWrite(13,HIGH);
  delay(500);
  digitalWrite(13,LOW);*/
    
}
void messageCbPan(const std_msgs::String& pan_msg){  
  pan = pan_msg.data[1];
}

ros::Subscriber<std_msgs::String> sub("toggle_led", &messageCb);
ros::Subscriber<std_msgs::String> subPan("pan", &messageCbPan);

std_msgs::String feedback;

ros::Publisher pub("feedback",&feedback);


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

void turn_right_90(){
  int i;
  for(i=0;i<30000;i++){
    set_motors(155, -155);
  }
}

void turn_left_90(){
  int i;
  for(i=0;i<30000;i++){
    set_motors(-155, 155);
  }
}

void turn_180(){
  for(int i=0;i<30000;i++){
    set_motors(155, -155);
  }
  for(int j=0;j<30000;j++){
    set_motors(155, -155);
  }
}

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
  nh.advertise(pub);
  
  pinMode(4,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(13,OUTPUT);
  
  myservo.attach(11);
  myservo.write(pos);
  
}

void loop(){
  
   if(state=='u')
   { 
     sent=false;
     if(once<1){
       set_motors(0,0); // Enquanto espera, motores permanecem parados
  
  
     //Serial.println("Auto-calibracao");
     // Auto-calibração: gira para a direita e depois esquerda e volta ao início
     // calibrando os sensores
 
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
  }
  
  else if(state=='m'){
    sent=false;
    fb="md";
    feedback.data=fb;
    follow_line();
    if(sensors[2]>500 && sensors[3]>500){
      for(int i=0; i<25000;i++){
        set_motors(180,180);
      }
      for(int i=0; i<25000;i++){
        set_motors(50,50);
      }
      for(int i=0; i<5000;i++){
        set_motors(0,0);
      }
      turn_180();
      pub.publish(&feedback);
      sent=true; 
      state='t';
    }
  } 
  
  else if(state=='i') 
  { 
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
  }
  
  else if(state=='e') 
  {
    sent=false;
    //feedback.data="left_done"; 
    for(counter=0;counter<50;counter++){
      set_motors(-30,30);
      delay(10);
    }
    state='t';
  }
  
  else if(state=='o')
  {
    sent=false;
    set_motors(80,80);
    /*digitalWrite(13,HIGH);
    delay(500);
    digitalWrite(13,LOW);
    delay(500);*/
  }
  
  else if(state=='a')
  {
    sent=false;
    set_motors(-50,-50);
  }
  else if(state=='t'){
    sent=false;
    if(!sent){
      fb="td";
      feedback.data=fb;
      pub.publish(&feedback);
      sent=true;
      state='z';
    }
    set_motors(0,0);
    pass=0;
    //digitalWrite(13,LOW);
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



