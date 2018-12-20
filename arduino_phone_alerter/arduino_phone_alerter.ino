//Key typer for SMS alerts
//Ultrasonic Sensor module triggers keys sequence
//Set up for Galaxy S3 with Messaging app as middle quick access icon
//Can set minimum time between alerts ALERTTIMEOUT in minutes
//Has key presser to keep screen awake (30s)
//Keyboard codes:https://www.arduino.cc/en/Reference/KeyboardModifiers  CTRL-ESC = home

#define ALERTTIMEOUT 10
#include "Keyboard.h"
#include "Mouse.h"
#define LEDPIN 13

// Ultrasonic HC-SR04 unit interface
//define pins here
//if using fixed power pins, set to a negative number, and they will be ignored
#define UVCC 10
#define UTRIG 11
#define UECHO 12
#define UGND 13

char kbuf[10]="";
int kptr=0;
unsigned long lastt=0;        //keep track of interval between alerts
unsigned long lastwake=0;     //keep track of interval between screen activity
long d;                       //for distance

void setup() {  
  delay(5000);          //let bootloader interrupt- allows sketches to upload easier
  usonicsetup();  
  Mouse.begin();
  Keyboard.begin();
  pinMode(LEDPIN,OUTPUT);     //LED
  digitalWrite(LEDPIN,LOW);   //LED off
}

void loop() {
  d=usonic(17400)/58;   //convert ping time to distance in cm
  if(d<15){             //too close!
    if((lastt==0)||(millis()-lastt>1000L*60*ALERTTIMEOUT)){ //if we haven't sent an alert, or more than 10min since last
      digitalWrite(LEDPIN,HIGH);                            //LED on
      sendalert();                                          //then send alert
      digitalWrite(LEDPIN,LOW);                             //LED off
      lastt=millis();                                       //reset timer
      lastwake=millis();                                    //reset activity timer too
    }
  }
  if(millis()-lastwake>1000L*30){keepawake();lastwake=millis();}         //do something to keep screen awake
  delay(200);                                               //wait between checks
}

void keepawake(){
  sendcmds(177);            //back, just some activity to keep screen awake
}

void sendalert(){
  byte cmds[]={
    177,177,177,177,        //back a few times, in case we're in something
    218,218,218,218,218,    //up a few times
    217,217,217,217,217,    //down a few times
    216,216,                //left twice
    176,                    //enter to messaging app
    218,                    //up to new message
    176,                    //enter
    0                       //so we know the end of list
  };
  sendcmds(cmds);
  Keyboard.print("xxxxxxxxxx");     //phone number field
  delay(2000);
  sendcmds(176);              //enter
  sendcmds(217);              //down to text field
  Keyboard.print("Sensor has been triggered! ");   //message text
  delay(2000);
  Keyboard.print(d);
  delay(2000);
  Keyboard.print(" cm.");   //message text  
  delay(2000);
  sendcmds(215);              //right to send button
  sendcmds(176);              //enter
}

void sendcmds(byte *c){       //outputs a list of commands
  while(*c){
    Keyboard.write(*c++);
    delay(1000);              //let it react
  }
}

void sendcmds(byte c){        //outputs a single command
  Keyboard.write(c);
  delay(1000);
}

void usonicsetup(void){
  if(UGND>-1){                  //set up ground pin if defined
    pinMode(UGND, OUTPUT);
    digitalWrite(UGND, LOW);    
  }
  if(UVCC>-1){                  //set up VCC pin if defined
    pinMode(UVCC, OUTPUT);
    digitalWrite(UVCC, HIGH);    
  }
  pinMode(UECHO, INPUT);        //ECHO pin is input
  pinMode(UTRIG, OUTPUT);       //TRIG pin is output
  digitalWrite(UTRIG, LOW);     
}

long usonic(long utimeout){    //utimeout is maximum time to wait for return in us
  long b;
  long utimer;
  if(digitalRead(UECHO)==HIGH){return utimeout;}    //if UECHO line is still low from last result
  digitalWrite(UTRIG, HIGH);  //send trigger pulse
  delayMicroseconds(10);
  digitalWrite(UTRIG, LOW);
  utimer=micros();
  while((digitalRead(UECHO)==LOW)&&((micros()-utimer)<1000)){}  //wait for pin state to change- return starts after 460us typically
  utimer=micros();
  delayMicroseconds(200);       //minimum range
  while((digitalRead(UECHO)==HIGH)&&((micros()-utimer)<utimeout)){}  //wait for pin state to change
  b=micros()-utimer;
  return b;
}


