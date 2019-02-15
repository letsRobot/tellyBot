// Tribot.c

//#define USING_360_SERVOS

#include "simpletools.h"
#include "fdserial.h"
#ifdef USING_360_SERVOS
  #include "servo360.h"
#else
  #include "servo.h"
#endif

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
int defaultStraightSpeed = 200;
int defaultTurnSpeed = 200;

//ARMS (3-Axis)
int storeArmStep = 20;
int armTimer = 100;
int armIndex = 3; //The size of the left / right arm array

//Left Arm Values
int leftArm[] =            {   11,   10,    9 }; //servos
int leftArmPosDefault[] =  {  400,  600, 1400 }; //calibrated to center postion
int leftArmMin[] =         {    0,    0,    0 };
int leftArmMax[] =         { 1800, 1800, 1800 };
int leftArmFlags[] =       {    0,    0,    0 }; //0: Do nothing, 1: move forward, 2: move backward
                                                 //Leave default flag values at 0
//Right Arm Values
int rightArm[] =           {   16,   18,   17 };
int rightArmPosDefault[] = { 2000, 1700,  200 };
int rightArmMin[] =        {    0,    0,    0 };
int rightArmMax[] =        { 2000, 1800, 1800 };
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

int centerPanTilt = 0;

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

        //Center PanTilt
        if (centerPanTilt == 1)
        {
            for (int i = 0; i < panTiltIndex; i++)
            {
                panTiltPos[i] = panTiltDefault[i];
                panTiltFlags[i] = 1;
            }
            centerPanTilt = 0;
        }

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

void StartTriBotController()
{
    cog_run(tri_motor_controller, 128);
    cog_run(moveArmsAndPanTilt, 128);
}

void HandleTriBotCommands(const char* inputString, fdserial *term)
{
    if (strcmp(inputString, "l") == 0)
    {
        dprint(term, "left");
        set_triBot(9);
    }
    if (strcmp(inputString, "r") == 0)
    {
        dprint(term, "right");
        set_triBot(8);
    }
    if (strcmp(inputString, "f") == 0)
    {
        dprint(term, "forward");
        set_triBot(0);
    }
    if (strcmp(inputString, "b") == 0)
    {
        dprint(term, "back");
        set_triBot(4);
    }
    if (strcmp(inputString, "a") == 0)
    {
        dprint(term, "slide left");
        set_triBot(6);
    }
    if (strcmp(inputString, "d") == 0)
    {
        dprint(term, "slide right");
        set_triBot(2);
    }
    if (strcmp(inputString, "e") == 0)
    {
        dprint(term, "forward-right");
        set_triBot(3);
    }
    if (strcmp(inputString, "q") == 0)
    {
        dprint(term, "forward-left");
        set_triBot(7);
    }
    if (strcmp(inputString, "c") == 0)
    {
        dprint(term, "backward-right");
        set_triBot(1);
    }
    if (strcmp(inputString, "z") == 0)
    {
        dprint(term, "backward-left");
        set_triBot(5);
    }
    if (strcmp(inputString, "s") == 0)
    {
        dprint(term, "stop");
        set_triBot(10);
    }
    if (strcmp(inputString, "x") == 0)
    {
        dprint(term, "stop with motors off");
        set_triBot(11);
    }

    //LEFT ARM
    if (strcmp(inputString, "g") == 0) {  leftArmFlags[0] = 1;  }
    if (strcmp(inputString, "o") == 0) {  leftArmFlags[0] = 2;  }
    if (strcmp(inputString, "y") == 0) {  leftArmFlags[1] = 1;  }
    if (strcmp(inputString, "u") == 0) {  leftArmFlags[1] = 2;  }
    if (strcmp(inputString, "v") == 0) {  leftArmFlags[2] = 1;  }
    if (strcmp(inputString, "w") == 0) {  leftArmFlags[2] = 2;  }


    //RIGHT ARM
    if (strcmp(inputString, "i") == 0) {  rightArmFlags[0] = 1; }
    if (strcmp(inputString, "h") == 0) {  rightArmFlags[0] = 2; }
    if (strcmp(inputString, "j") == 0) {  rightArmFlags[1] = 1; }
    if (strcmp(inputString, "k") == 0) {  rightArmFlags[1] = 2; }
    if (strcmp(inputString, "m") == 0) {  rightArmFlags[2] = 1; }
    if (strcmp(inputString, "n") == 0) {  rightArmFlags[2] = 2; }

    //PAN AND TILT
    if (strcmp(inputString, "pp") == 0) { panTiltFlags[0] = 1;  }
    if (strcmp(inputString, "pm") == 0) { panTiltFlags[0] = 2;  }
    if (strcmp(inputString, "tp") == 0) { panTiltFlags[1] = 1;  }
    if (strcmp(inputString, "tm") == 0) { panTiltFlags[1] = 2;  }

    if (strcmp(inputString, "center") == 0) { centerPanTilt = 1;  }
}

