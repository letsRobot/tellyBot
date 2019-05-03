// Tribot.c
#include "BotConfig.h"

#if (CONFIGURED_BOT == TRI_BOT)

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
int motorA[] = {   0, -8, -8, -6,   0,   6,  8,   6,  8, -8, 11};
int motorB[] = { -14, 12,  5,  2,  14,   2, -5, -12,  8, -8, 11};
int motorC[] = {  14, -2,  6, 12, -14, -12, -6,   2,  8, -8, 11};

//General Speed Controls
int defaultStraightSpeed = DEFAULT_STRAIGHT_SPEED;
int defaultTurnSpeed = DEFAULT_TURN_SPEED;

int pivotFlag = 0;

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

void StartTriBotController()
{
    cog_run(tri_motor_controller, 128);
}

void HandleTriBotCommands(const char* inputString, fdserial *term)
{
    if (strcmp(inputString, "l") == 0 )
    {
        dprint(term, "left");
        set_triBot(8);
    }
    if (strcmp(inputString, "r") == 0 )
    {
        dprint(term, "right");
        set_triBot(9);
    }
    if (strcmp(inputString, "f") == 0 && pivotFlag == 0)
    {
        dprint(term, "forward");
        set_triBot(0);
    }
    if (strcmp(inputString, "b") == 0 && pivotFlag == 0)
    {
        dprint(term, "back");
        set_triBot(4);
    }
    if (strcmp(inputString, "a") == 0 && pivotFlag == 0)
    {
        dprint(term, "slide left");
        set_triBot(2);
    }
    if (strcmp(inputString, "d") == 0 && pivotFlag == 0)
    {
        dprint(term, "slide right");
        set_triBot(6);
    }
    if (strcmp(inputString, "e") == 0 && pivotFlag == 0)
    {
        dprint(term, "forward-right");
        set_triBot(7);
    }
    if (strcmp(inputString, "q") == 0 && pivotFlag == 0)
    {
        dprint(term, "forward-left");
        set_triBot(3);
    }
    if (strcmp(inputString, "c") == 0 && pivotFlag == 0)
    {
        dprint(term, "backward-right");
        set_triBot(5);
    }
    if (strcmp(inputString, "z") == 0 && pivotFlag == 0)
    {
        dprint(term, "backward-left");
        set_triBot(1);
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
    
    if (strcmp(inputString, "pivot") == 0)
    { 
        pivotFlag = !pivotFlag;
        dprint(term, "pivot %s", pivotFlag ? "on" : "off");
    }
    
    if (strncmp(inputString, "speed", 5) == 0) 
    { 
        const char* pBeg = &inputString[0];
        char* pEnd;
        int new_speed = strtol(pBeg+6, &pEnd,10);
        if ( new_speed < 10 ) new_speed = 10;
        if ( new_speed > 200 ) new_speed = 200;
        defaultStraightSpeed = new_speed;
        dprint(term,"speed set to %d", new_speed);
    }
}

#endif // (CONFIGURED_BOT == TRI_BOT)