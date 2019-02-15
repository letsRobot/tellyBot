// StandardBot.c

//#define USING_360_SERVOS

#include "simpletools.h"
#include "fdserial.h"
#ifdef USING_360_SERVOS
  #include "servo360.h"
#else
  #include "servo.h"
#endif  
#include "abdrive360.h"

// General Speed Controls
int _standardBotDefaultStraightSpeed = 200;
int _standardBotDefaultTurnSpeed = 200;

// used to communicate to the motor_controller in another core
volatile int _standard_bot_current_leftspd = 0;
volatile int _standard_bot_current_rightspd = 0;
volatile int _standard_bot_motor_flag = 0;

void set_motor_controller(int leftSpeed, int rightSpeed)
{
    _standard_bot_current_leftspd = leftSpeed;
    _standard_bot_current_rightspd = rightSpeed;
    _standard_bot_motor_flag = 1;
}  

void Stop(void)
{
    drive_speed(0, 0);
}  

void Step(int leftSpeed, int rightSpeed)
{
#ifdef NO_ENCODERS
    drive_speeds(leftSpeed, rightSpeed);
#else
    drive_speed(leftSpeed, rightSpeed);
#endif
}  

void motor_controller()
{
    uint32_t last_ms = 0;
    uint32_t current_ms = 0;
    uint32_t wait_ms = 10;
    uint32_t clk_wait = 80000*wait_ms;
    uint32_t timeout_timer = 0;
    uint32_t timeout_ms = 80000*500;
 
    while (1)
    {

        current_ms = CNT;  
        if (current_ms-last_ms >= clk_wait)
        {            
            last_ms = current_ms;
            if (_standard_bot_motor_flag == 1) 
            {
                Step(_standard_bot_current_leftspd,_standard_bot_current_rightspd);
                _standard_bot_motor_flag = 0;
                timeout_timer = current_ms;
            }
            if (current_ms - timeout_timer >= timeout_ms)
            {
                Stop();
            }   
         
        }        
    }  
}

void StartStandardBotController()
{
    cog_run(motor_controller, 128);
}

void HandleStandardBotCommands(const char* inputString, fdserial *term)
{
    if (strcmp(inputString, "l") == 0)
    {
        dprint(term, "left");
        set_motor_controller(-_standardBotDefaultTurnSpeed,_standardBotDefaultTurnSpeed);
    }          
    if (strcmp(inputString, "r") == 0)
    {
        dprint(term, "right");
        set_motor_controller(_standardBotDefaultTurnSpeed, -_standardBotDefaultTurnSpeed);
    }          
    if (strcmp(inputString, "f") == 0)
    {
        dprint(term, "forward");
        set_motor_controller(_standardBotDefaultStraightSpeed, _standardBotDefaultStraightSpeed);
    }          
    if (strcmp(inputString, "b") == 0)
    {
        dprint(term, "back");
        set_motor_controller(-_standardBotDefaultStraightSpeed, -_standardBotDefaultStraightSpeed);
    }
    if (strcmp(inputString, "l_up") == 0)
    {
        dprint(term, "left_stop");
        Stop();
    }          
    if (strcmp(inputString, "r_up") == 0)
    {
        dprint(term, "right_stop");
        Stop();
    }          
    if (strcmp(inputString, "f_up") == 0)
    {
        dprint(term, "forward_stop");
        Stop();
    }          
    
    if (strcmp(inputString, "b_up") == 0)
    {
        dprint(term, "back_stop");
        Stop();
    }
}

