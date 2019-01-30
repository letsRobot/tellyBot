/**
* Going to slowly move stuff out of this file and use telly.c as the main file. 
*/

//#define USING_360_SERVOS
#define TRI_BOT

#include "simpletools.h"
#include "fdserial.h"
#include "abdrive360.h"
#include "ping.h"
#ifdef USING_360_SERVOS
  #include "servo360.h"
#else
  #include "servo.h"
#endif  
#include "ws2812.h"

void Step(int leftSpeed, int rightSpeed);
void Stop(void);
void led_blink();
void eyes_blink();
void motor_controller();
void neopixel_controller();
void set_motor_controller(int leftSpeed, int rightSpeed);
void set_neopixel_group(uint32_t color);
void set_neopixel(uint8_t pixel_num, uint32_t color);
void pause(int ms);
void refresh_eyes();
void increase_brightness();
void decrease_brightness();
void drive_triBot(int idx);
void set_triBot(int idx);
void tri_motor_controller();
void handle_eyes();
void moveArmsAndPanTilt();
void moveLeftArm();
void moveRightArm();
void movePanTilt();

#define LED_PIN     8
#define LED_COUNT   18
ws2812 *led_driver;
int ticks_per_ms;
  
// used by handle_eyes function
volatile int update_eyes = 0;
volatile uint8_t pixel = 0;
volatile uint32_t pixel_color = 0xFFFFFF;
volatile uint8_t brightness = 10;
volatile uint32_t eye_color = 0xFFFFFF;
volatile uint32_t ledColors[LED_COUNT];
volatile int eye_command = 0;

uint32_t dim_array[LED_COUNT];

#define EYES_BLINK 1
#define EYES_INCREASE_BRIGHTNESS 2
#define EYES_DECREASE_BRIGHTNESS 3
#define EYES_SET_COLOR_SINGLE_PIXEL 4
#define EYES_SET_COLOR_ALL_PIXELS 5

fdserial *term; //enables full-duplex serilization of the terminal (In otherwise, 2 way signals between this computer and the robot)
int ticks = 12; //each tick makes the wheel move by 3.25mm, 64 ticks is a full wheel rotation (or 208mm)
int turnTick = 6; //Turning is at half the normal rate
int maxSpeed = 128; //the maximum amount of ticks the robot can travel with the "drive_goto" function
int minSpeed = 2; //the lowest amount of ticks the robot can travel with the "drive_goto" function
int maxTurnSpeed = 64;
int minTurnSpeed = 2;
int gripDegree = 0; //Angle of the servo that controls the gripper
int gripState = -1;
int commandget = 0;

//TriBot paramaters 
volatile int current_leftspd = 0;
volatile int current_rightspd = 0;
volatile int cmd_dir = 11;  // stopped
volatile int motor_flag = 0;

//              FORWARD          REVERSE
//                 0   1   2   3    4    5   6    7   R   L  S    (Disable == 11)
int motorA[] = {   0, -8, -8, -6,   0,   6,  8,   6,  4, -4, 0};
int motorB[] = { -14, 12,  5,  2,  14,   2, -5, -12,  4, -4, 0};
int motorC[] = {  14, -2,  6, 12, -14, -12, -6,   2,  4, -4, 0};

//General Speed Controls
int defaultStraightSpeed = 150;
int defaultTurnSpeed = 200;

int pingDistance;

//ARMS (3-Axis)
int storeArmStep = 20;
int armTimer = 100;
int armIndex = 3; //The size of the left / right arm array

//Left Arm Values
int leftArm[] =            {   11,   10,    9 }; //servos
int leftArmPosDefault[] =  {  900,  990,  950 }; //calibrated to center postion
int leftArmMin[] =         {  200,  200,  200 }; 
int leftArmMax[] =         { 1600, 1600, 1600 }; 
int leftArmFlags[] =       {    0,    0,    0 }; //0: Do nothing, 1: move forward, 2: move backward
                                                 //Leave default flag values at 0
//Right Arm Values
int rightArm[] =           {   16,   18,   17 };
int rightArmPosDefault[] = {  990,  990,  900 };
int rightArmMin[] =        {  200,  200,  200 }; 
int rightArmMax[] =        { 1600, 1600, 1600 }; 
int rightArmFlags[] =      {    0,    0,    0 };
  
//pan and tilt values
int storePanTiltStep = 20;
int panTiltTimer = 100;
int panTiltIndex = 2;
int panTilt[] =            {   19,   20 };
int panTiltDefault[] =     {  900,  900 };
int panTiltMin[] =         {  100,  100 };
int panTiltMax[] =         { 1700, 1700 };
int panTiltFlags[] =       {    0,    0 }; 
int panTiltState = 0; //reserved to override default behavior



/*
Character Assignments for key commands

a   Tribot    Strafe Left
b   Default   Backward
c   Tribot    Backward Right
d   Tribot    Strafe Right
e   Tribot    Forward Right
f   Default   Forward
g   TriBot    Right Shoulder 1 Foward
h   Tribot    Left Shoulder 1 Back
i   Tribot    Left Shoulder 1 Forward
j   Tribot    Left Shoulder 2 Forward
k   Tribot    Left Shoulder 2 Back
l   Default   Rotate Left
m   Tribot    Left Shoulder 3 Forward
n   Tribot    Left Shoulder 3 Back
o   Tribot    Right Shoulder 1 Back
p   Reserved
q   Tribot    Forward Left
r   Default   Rotate Right
s   Tribot    Stop
t   Reserved
u   Tribot    Right Shoulder 2 Back
v   Tribot    Right Shoulder 3 Forward
w   Tribot    Right Shoulder 3 Back
x   Tribot    Stop with motors off
y   Tribot    Right Shoulder 2 Forward
z   Tribot    Backward Left

*/

int main()
{

    //access the simpleIDE terminal
    simpleterm_close();
    //set full-duplex serialization for the terminal
    term = fdserial_open(31, 30, 0, 9600);

    ticks_per_ms = CLKFREQ / 1000;

#ifdef TRI_BOT
    cog_run(tri_motor_controller,128);
    cog_run(moveArmsAndPanTilt, 128);
    //cog_run(moveLeftArm, 128);
    //cog_run(moveRightArm, 128);
    //cog_run(movePanTilt, 128);
    
#else
    cog_run(motor_controller,128);
#endif

    // load the LED driver
    if (!(led_driver = ws2812b_open()))
        return 1;

    pause(250);
    cog_run(handle_eyes, 128);

    pause(100);
    update_eyes = EYES_BLINK;   



    char c;  
    int inputStringLength = 20;
    char inputString[inputStringLength];
    int sPos = 0;

    while (1)
    {
        if (fdserial_rxReady(term)!=0)
        {
            c = fdserial_rxChar(term); //Get the character entered from the terminal

            if (c != -1)
            {
                dprint(term, "%d", (int)c);
                if ((int)c == 13 || (int)c == 10)
                {
                    dprint(term, "received line:");
                    dprint(term, inputString);
                    dprint(term, "\n");
     
#ifndef TRI_BOT
            
                    if (strcmp(inputString, "l") == 0) {
                        dprint(term, "left");
                        set_motor_controller(-defaultTurnSpeed,defaultTurnSpeed);
                    }          
                    if (strcmp(inputString, "r") == 0) {
                        dprint(term, "right");
                        set_motor_controller(defaultTurnSpeed, -defaultTurnSpeed);
                    }          
                    if (strcmp(inputString, "f") == 0) {
                        dprint(term, "forward");
                        set_motor_controller(defaultStraightSpeed, defaultStraightSpeed);
                    }          
                    if (strcmp(inputString, "b") == 0) {
                        dprint(term, "back");
                        set_motor_controller(-defaultStraightSpeed, -defaultStraightSpeed);
                    }
                    if (strcmp(inputString, "l_up") == 0) {
                        dprint(term, "left_stop");
                        Stop();
                    }          
                    if (strcmp(inputString, "r_up") == 0) {
                        dprint(term, "right_stop");
                        Stop();
                    }          
                    if (strcmp(inputString, "f_up") == 0) {
                        dprint(term, "forward_stop");
                        Stop();
                    }          
                    
                    if (strcmp(inputString, "b_up") == 0) {
                        dprint(term, "back_stop");
                        Stop();
                    }

#else

                    if (strcmp(inputString, "l") == 0) {
                        dprint(term, "left");
                        set_triBot(9);
                    }          
                    if (strcmp(inputString, "r") == 0) {
                        dprint(term, "right");
                        set_triBot(8);
                    }          
                    if (strcmp(inputString, "f") == 0) {
                        dprint(term, "forward");
                        set_triBot(0);
                    }          
                    if (strcmp(inputString, "b") == 0) {
                        dprint(term, "back");
                        set_triBot(4);
                    }          
                    if (strcmp(inputString, "a") == 0) {
                        dprint(term, "slide left");
                        set_triBot(6);
                    }          
                    if (strcmp(inputString, "d") == 0) {
                        dprint(term, "slide right");
                        set_triBot(2);
                    }          
                    if (strcmp(inputString, "e") == 0) {
                        dprint(term, "forward-right");
                        set_triBot(3);
                    }          
                    if (strcmp(inputString, "q") == 0) {
                        dprint(term, "forward-left");
                        set_triBot(7);
                    }          
                    if (strcmp(inputString, "c") == 0) {
                        dprint(term, "backward-right");
                        set_triBot(1);
                    }          
                    if (strcmp(inputString, "z") == 0) {
                        dprint(term, "backward-left");
                        set_triBot(5);
                    }          
                    if (strcmp(inputString, "s") == 0) {
                        dprint(term, "stop");
                        set_triBot(10);
                    }          
                    if (strcmp(inputString, "x") == 0) {
                        dprint(term, "stop with motors off");
                        set_triBot(11);
                    }  
                    
                    //LEFT ARM
                    if (strcmp(inputString, "i") == 0) {  leftArmFlags[0] = 1;  }
                    if (strcmp(inputString, "h") == 0) {  leftArmFlags[0] = 2;  }
                    if (strcmp(inputString, "j") == 0) {  leftArmFlags[1] = 1;  }
                    if (strcmp(inputString, "k") == 0) {  leftArmFlags[1] = 2;  } 
                    if (strcmp(inputString, "m") == 0) {  leftArmFlags[2] = 1;  }
                    if (strcmp(inputString, "n") == 0) {  leftArmFlags[2] = 2;  }     
                    
                    
                    //RIGHT ARM
                    if (strcmp(inputString, "g") == 0) {  rightArmFlags[0] = 1; }  
                    if (strcmp(inputString, "o") == 0) {  rightArmFlags[0] = 2; } 
                    if (strcmp(inputString, "y") == 0) {  rightArmFlags[1] = 1; }  
                    if (strcmp(inputString, "u") == 0) {  rightArmFlags[1] = 2; } 
                    if (strcmp(inputString, "v") == 0) {  rightArmFlags[2] = 1; }  
                    if (strcmp(inputString, "w") == 0) {  rightArmFlags[2] = 2; }     
                    
                    //PAN AND TILT
                    if (strcmp(inputString, "pp") == 0) { panTiltFlags[0] = 1;  }  
                    if (strcmp(inputString, "pm") == 0) { panTiltFlags[0] = 2;  }  
                    if (strcmp(inputString, "tp") == 0) { panTiltFlags[1] = 1;  }  
                    if (strcmp(inputString, "tm") == 0) { panTiltFlags[1] = 2;  } 
            
#endif
            
                    if (strcmp(inputString, "brightness_up") == 0) {
                        update_eyes = EYES_INCREASE_BRIGHTNESS;
                        dprint(term,"brightness increased");
                    }
     
                    if (strcmp(inputString, "brightness_down") == 0) {
                        update_eyes = EYES_DECREASE_BRIGHTNESS;
                        dprint(term,"brightness decreased");
                    }
              
                    if (strncmp(inputString, "led",3) == 0) 
                    { 
                        char * pBeg = &inputString[0];
                        char * pEnd;
                        pixel = strtol(pBeg+4, &pEnd,10);
                        pixel_color = strtol(pEnd, &pEnd,16);
                        dprint(term,"%d\n",pixel_color);
                        if((pixel < LED_COUNT)&&(pixel_color<=0xFFFFFF))
                            update_eyes = EYES_SET_COLOR_SINGLE_PIXEL;
                    }           

                    if (strncmp(inputString, "leds",4) == 0) 
                    { 
                        char * pBeg = &inputString[0];
                        char * pEnd;
                        pixel_color = strtol(pBeg+5, &pEnd,16);
                        dprint(term,"%d\n",pixel_color);
                        if(pixel_color<=0xFFFFFF)
                            update_eyes = EYES_SET_COLOR_ALL_PIXELS;
                    }                   
                    sPos = 0;
                    inputString[0] = 0; // clear string
                }
                else if (sPos < inputStringLength - 1)
                {
                    // record next character
                    inputString[sPos] = c;
                    sPos += 1;
                    inputString[sPos] = 0; // make sure last element of string is 0
                    dprint(term, inputString);
                    dprint(term, " ok \n");
                }  
            }            
        }      
    }   
}

void pause(int ms)
{
    waitcnt(CNT + ms * ticks_per_ms);
}

void led_blink()                             // Blink function for other cog
{
    while(1)                                 // Endless loop for other cog
    {
        high(26);                            // P26 LED on
        pause(1000);                         // ...for 0.1 seconds
        low(26);                             // P26 LED off
        pause(1000);                         // ...for 0.1 seconds
    }
}

void Step(int leftSpeed, int rightSpeed)
{
#ifdef NO_ENCODERS
    drive_speeds(leftSpeed, rightSpeed);
#else
    drive_speed(leftSpeed, rightSpeed);
#endif
}  

void set_motor_controller(int leftSpeed, int rightSpeed)
{
    current_leftspd =leftSpeed;
    current_rightspd = rightSpeed;
    motor_flag = 1;
}  

void Stop(void)
{
    drive_speed(0, 0);
}  

void motor_controller()
{
    uint32_t last_ms = 0;
    uint32_t current_ms = 0;
    uint32_t wait_ms = 10;
    uint32_t clk_wait = 80000*wait_ms;
    uint32_t timeout_timer = 0;
    uint32_t timeout_ms = 80000*500;
 
    while(1)
    {

        current_ms = CNT;  
        if(current_ms-last_ms >= clk_wait )
        {            
            last_ms = current_ms;
            if(motor_flag == 1) 
            {
                Step(current_leftspd,current_rightspd);
                motor_flag =0;
                timeout_timer = current_ms;
            }
            if(current_ms-timeout_timer >= timeout_ms)
            {
                Stop();
            }   
         
        }        
    }  
}

void handle_eyes()
{
    while(1)
    {
        if(update_eyes > 0)
        {
            // I'm doing this local copy of the update_eyes variable so that I can clear update_eyes variable right away
            // this makes it so that if you get another eye command while it's still in the middle of doing the last one
            // it will do that new one after.  This only allows one queued up command, but that should be fine for most cases
            eye_command = update_eyes;
            update_eyes = 0;
            switch(eye_command)
            {
                case EYES_BLINK:
                    eyes_blink();
                    break;
                case EYES_INCREASE_BRIGHTNESS:
                    increase_brightness();
                    break;
                case EYES_DECREASE_BRIGHTNESS:
                    decrease_brightness();
                    break;
                case EYES_SET_COLOR_SINGLE_PIXEL:
                    set_neopixel(pixel, pixel_color);
                    break;
                case EYES_SET_COLOR_ALL_PIXELS:
                    set_neopixel_group(pixel_color);
                    break;
            }
        }
    }
}

void refresh_eyes()
{
    int i2;

    for (i2 = 0; i2 < LED_COUNT; ++i2)
    {
        uint32_t red = (ledColors[i2]>>16) & 0xFF;
        red = red*brightness/255;
        uint32_t green = (ledColors[i2]>>8) & 0xFF;
        green = green*brightness/255;
        uint32_t blue = (ledColors[i2]) & 0xFF;
        blue = blue*brightness/255;
        uint32_t scaled_color = (red << 16)+(green << 8)+(blue);
        dim_array[i2] = scaled_color;      
    }   

    ws2812_set(led_driver, LED_PIN, dim_array, LED_COUNT);
}  

void increase_brightness()
{
    int16_t temp_brightness = brightness+10;
    if(temp_brightness>255)
    {
        temp_brightness = 255;
    } 
    brightness = temp_brightness;
    refresh_eyes();
}  

void decrease_brightness()
{
    int16_t temp_brightness = brightness-10;
    if(temp_brightness<=1)
    {
        temp_brightness = 2;
    } 
    brightness = temp_brightness;
    refresh_eyes();
} 

void set_neopixel(uint8_t pixel_num, uint32_t color)
{
    if(pixel_num <LED_COUNT)
    {
        ledColors[pixel_num] = color;
    }
    refresh_eyes();
}

void set_neopixel_group(uint32_t color)
{
    int i;
    for (i = 0; i < LED_COUNT; ++i)
    {
        ledColors[i] = color;
    }   
    refresh_eyes();
}

void eyes_blink()
{
    int doot;
    doot=0;
    while(doot<LED_COUNT)
    {
        if(doot==4||doot==13)
            set_neopixel(doot,0x000000);  
        else
            set_neopixel(doot,eye_color);         
        doot+=1;
        pause(1);
    }     
    doot =0;   
    pause(400);
    while(doot<LED_COUNT)
    {
        if((doot>=3 && doot<=5)|| (doot>=12 && doot<=14))
            set_neopixel(doot,eye_color);  
        else
            set_neopixel(doot,0x000000);         
        doot+=1;
        pause(1);
    }     
    doot =0; 
    pause(400);
    while(doot<LED_COUNT)
    {
        if(doot==4||doot==13)
            set_neopixel(doot,0x000000);  
        else
            set_neopixel(doot,eye_color);         
        doot+=1;
        pause(1);
    }
}  


void drive_triBot(int idx)
{
    if(idx != 11)
    {
        int times_speed = defaultStraightSpeed / 11;
        servo_speed(12, motorA[idx] * times_speed); 
        servo_speed(13, motorB[idx] * times_speed);
        servo_speed(14, motorC[idx] * times_speed);
    }
    else
    {
        servo_disable(12);
        servo_disable(13);
        servo_disable(14);
    }      
}  

void set_triBot(int idx)
{
    cmd_dir = idx;
    motor_flag = 1;
}  

void tri_motor_controller()
{
    uint32_t last_ms = 0;
    uint32_t current_ms = 0;
    uint32_t wait_ms = 10;
    uint32_t clk_wait = 80000*wait_ms;
    uint32_t timeout_timer = 0;
    uint32_t timeout_ms = 80000*500;
 
    while(1)
    {

        current_ms = CNT;  
        if(current_ms-last_ms >= clk_wait )
        {            
            last_ms = current_ms;
            if(motor_flag == 1) 
            {
                drive_triBot(cmd_dir);
                motor_flag =0;
                timeout_timer = current_ms;
            }
            if(current_ms-timeout_timer >= timeout_ms)
            {
                drive_triBot(11);  //stop
            }         
        }        
    }  
}


//Technically, this is more of a servo mover, and i can probably refactor to a more generic system

void moveArmsAndPanTilt()
{
  int armStep = storeArmStep;

  int panTiltStep = storePanTiltStep;
  
  //use calibrated defaults for starting position
  int panTiltPos[] = {panTiltDefault[0], panTiltDefault[1]};

  //initialize arms into default pose
  for (int i = 0; i < panTiltIndex; i++)
  {
    servo_angle(panTilt[i], panTiltPos[i]);
  }    
  
  //use calibrated defaults for starting position
  int leftArmPos[] = {leftArmPosDefault[0], leftArmPosDefault[1], leftArmPosDefault[2]};

  //use calibrated defaults for starting position
  int rightArmPos[] = {rightArmPosDefault[0], rightArmPosDefault[1], rightArmPosDefault[2]};

  //initialize arms into default pose
  for (int i = 0; i < armIndex; i++)
  {
    servo_angle(rightArm[i], rightArmPos[i]);
    servo_angle(leftArm[i], leftArmPos[i]);
  }    

  while(1)
  {
    for (int i = 0; i < panTiltIndex; i++)
    {
       if (panTiltFlags[i] != 0 )
       {
         if (panTiltFlags[i] == 2 )
         {
           panTiltPos[i] -= panTiltStep;
           if (panTiltPos[i] < panTiltMin[i])
           {
             panTiltPos[i] = panTiltMin[i]; 
           }           
         }
         else
         {
           panTiltPos[i] += panTiltStep;
           if (panTiltPos[i] > panTiltMax[i])
           {
             panTiltPos[i] = panTiltMax[i]; 
           }           
         }          
         servo_angle(panTilt[i], panTiltPos[i]);
         pause(panTiltTimer);
         panTiltFlags[i] = 0;
      }        
    }        
    for (int i = 0; i < armIndex; i++)
    {
       if (leftArmFlags[i] != 0 )
       {
         if (leftArmFlags[i] == 2 )
         {
           leftArmPos[i] -= armStep;
           if (leftArmPos[i] < leftArmMin[i])
           {
             leftArmPos[i] = leftArmMin[i]; 
           }           
         }
         else
         {
           leftArmPos[i] += armStep;
           if (leftArmPos[i] > leftArmMax[i])
           {
             leftArmPos[i] = leftArmMax[i]; 
           }           
         }          
         servo_angle(leftArm[i], leftArmPos[i]);
         pause(armTimer);
         leftArmFlags[i] = 0;
       }        
       if (rightArmFlags[i] != 0 )
       {
         if (rightArmFlags[i] == 2 )
         {
           rightArmPos[i] -= armStep;
           if (rightArmPos[i] < rightArmMin[i])
           {
             rightArmPos[i] = rightArmMin[i]; 
           }           
         }
         else
         {
           rightArmPos[i] += armStep;
           if (rightArmPos[i] > rightArmMax[i])
           {
             rightArmPos[i] = rightArmMax[i]; 
           }           
         }          
         servo_angle(rightArm[i], rightArmPos[i]);
         pause(armTimer);
         rightArmFlags[i] = 0;
       }        
    }        
    pause(10);
  }                  
} 


void moveLeftArm() {
  int armStep = storeArmStep;
  //use calibrated defaults for starting position
  int leftArmPos[] = {leftArmPosDefault[0], leftArmPosDefault[1], leftArmPosDefault[2]};
 

  //initialize arms into default pose
  for (int i = 0; i < armIndex; i++) {
    servo_angle(leftArm[i], leftArmPos[i]);
  }    

  while(1) {
    
    for (int i = 0; i < armIndex; i++) {
       if (leftArmFlags[i] != 0 ) {
         if (leftArmFlags[i] == 2 ) {
          leftArmPos[i] -= armStep;
         if (leftArmPos[i] < leftArmMin[i]) {
          leftArmPos[i] = leftArmMin[i]; 
         }           
       } else {
         leftArmPos[i] += armStep;
         if (leftArmPos[i] > leftArmMax[i]) {
          leftArmPos[i] = leftArmMax[i]; 
         }           
       }          
         servo_angle(leftArm[i], leftArmPos[i]);
         pause(armTimer);
         leftArmFlags[i] = 0;
      }        
    }        
    pause(10);
  }                  
} 

void moveRightArm() {
  int armStep = storeArmStep;
  //use calibrated defaults for starting position
  int rightArmPos[] = {rightArmPosDefault[0], rightArmPosDefault[1], rightArmPosDefault[2]};


  //initialize arms into default pose
  for (int i = 0; i < armIndex; i++) {
    servo_angle(rightArm[i], rightArmPos[i]);
  }    

  while(1) {

    for (int i = 0; i < armIndex; i++) {
       if (rightArmFlags[i] != 0 ) {
         if (rightArmFlags[i] == 2 ) {
          rightArmPos[i] -= armStep;
         if (rightArmPos[i] < rightArmMin[i]) {
          rightArmPos[i] = rightArmMin[i]; 
         }           
       } else {
         rightArmPos[i] += armStep;
         if (rightArmPos[i] > rightArmMax[i]) {
          rightArmPos[i] = rightArmMax[i]; 
         }           
       }          
         servo_angle(rightArm[i], rightArmPos[i]);
         pause(armTimer);
         rightArmFlags[i] = 0;
      }        
    }        
    pause(10);
  }                  
} 

void movePanTilt() {
  int step = storePanTiltStep;
  //use calibrated defaults for starting position
  int panTiltPos[] = {panTiltDefault[0], panTiltDefault[1]};


  //initialize arms into default pose
  for (int i = 0; i < panTiltIndex; i++) {
    servo_angle(panTilt[i], panTiltPos[i]);
  }    

  while(1) {

    for (int i = 0; i < panTiltIndex; i++) {
       if (panTiltFlags[i] != 0 ) {
         if (panTiltFlags[i] == 2 ) {
          panTiltPos[i] -= step;
         if (panTiltPos[i] < panTiltMin[i]) {
          panTiltPos[i] = panTiltMin[i]; 
         }           
       } else {
         panTiltPos[i] += step;
         if (panTiltPos[i] > panTiltMax[i]) {
          panTiltPos[i] = panTiltMax[i]; 
         }           
       }          
         servo_angle(panTilt[i], panTiltPos[i]);
         pause(panTiltTimer);
         panTiltFlags[i] = 0;
      }        
    }        
    pause(10);
  }                  
} 



void flappyBot() {
  
  servo_angle(leftArm[2], 600);
  servo_angle(rightArm[2], 1300);
  pause(500);
  servo_angle(leftArm[2], 1400);
  servo_angle(rightArm[2], 300);
  pause(500);

}  