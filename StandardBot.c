// StandardBot.c
#include "BotConfig.h"

#if (CONFIGURED_BOT == STANDARD_BOT)

#include "simpletools.h"
#include "fdserial.h"
#ifdef USING_360_SERVOS
  #include "servo360.h"
  #include "abdrive360.h"
#else
  #include "servo.h"
  #include "abdrive.h"
#endif  

// General Speed Controls
int _standardBotDefaultStraightSpeed = DEFAULT_STRAIGHT_SPEED;
int _standardBotDefaultTurnSpeed = DEFAULT_TURN_SPEED;
int _standardBotDefaultStraightTimeout = DEFAULT_STRAIGHT_TIMEOUT;
int _standardBotDefaultTurnTimeout = DEFAULT_TURN_TIMEOUT;

volatile int straightSpeed;
volatile int turnSpeed;

//Used for speed boost!
volatile int boostFlag = 0;

// used to communicate to the motor_controller in another core
volatile int _standard_bot_current_leftspd = 0;
volatile int _standard_bot_current_rightspd = 0;
volatile int _standard_bot_current_timeout = 0;
volatile int _standard_bot_motor_flag = 0;

void set_motor_controller(int leftSpeed, int rightSpeed, int timeout)
{
    _standard_bot_current_leftspd = leftSpeed;
    _standard_bot_current_rightspd = rightSpeed;
    _standard_bot_current_timeout = timeout;
    _standard_bot_motor_flag = 1;
}  

void Stop(void)
{
    drive_speed(0, 0);
}  

void Go(int leftSpeed, int rightSpeed)
{  
    drive_speed(leftSpeed, rightSpeed);
}

void motor_controller() 
{
    int timeout = 0;
    while (1) 
    {
        if (_standard_bot_motor_flag == 1)
        {
            Stop();
            Go(_standard_bot_current_leftspd, _standard_bot_current_rightspd);
            _standard_bot_motor_flag = 0;
            timeout = _standard_bot_current_timeout;
        }
        else if ( --timeout <= 0 )
        {
            Stop();
            timeout = 0;
        }
        pause(10);
    }
}


void speedBoost()
{
    int boostTimeout = 0;
    while (1)
    {  
        if (boostFlag == 1)
        {
            boostTimeout += (BOOST_TIME_SECONDS * 100); // scale seconds to 10ms units
            straightSpeed = BOOST_STRAIGHT_SPEED;
            turnSpeed = BOOST_TURN_SPEED;
            boostFlag = 0;
      
        }
        else if (--boostTimeout <= 0)
        {
            boostTimeout = 0;
            straightSpeed = _standardBotDefaultStraightSpeed;
            turnSpeed = _standardBotDefaultTurnSpeed;  
        } 
        pause(10);               
    }  
}

void StartStandardBotController()
{
#ifdef USING_360_SERVOS
    servo360_couple(12, 13);
    servo360_setCoupleScale(12, 13, 2000);
#endif

    straightSpeed = _standardBotDefaultStraightSpeed;
    turnSpeed = _standardBotDefaultTurnSpeed;
    
    cog_run(motor_controller, 128);
    cog_run(speedBoost,128);
}

void HandleStandardBotCommands(const char* inputString, fdserial *term)
{
    if (strcmp(inputString, "l") == 0)
    {
        dprint(term, "left");
        set_motor_controller(-turnSpeed, turnSpeed, _standardBotDefaultTurnTimeout);
    }          
    if (strcmp(inputString, "r") == 0)
    {
        dprint(term, "right");
        set_motor_controller(turnSpeed, -turnSpeed, _standardBotDefaultTurnTimeout);
    }          
    if (strcmp(inputString, "f") == 0)
    {
        dprint(term, "forward");
        set_motor_controller(straightSpeed, straightSpeed, _standardBotDefaultStraightTimeout);
    }          
    if (strcmp(inputString, "b") == 0)
    {
        dprint(term, "back");
        set_motor_controller(-straightSpeed, -straightSpeed, _standardBotDefaultStraightTimeout);
    }
    if (strcmp(inputString, "boost") == 0)
    {
        dprint(term, "Speed Boost!");
        //boost for duration of boost timer
        boostFlag = 1;                        
    }
    
    if (strncmp(inputString, "speed", 5) == 0) 
    { 
        const char* pBeg = &inputString[0];
        char* pEnd;
        int new_speed = strtol(pBeg+6, &pEnd,10);
        if ( new_speed < 10 ) new_speed = 10;
        if ( new_speed > 200 ) new_speed = 200;
        _standardBotDefaultStraightSpeed = new_speed;
        dprint(term,"straight speed set to %d", new_speed);
    }
    
    if (strncmp(inputString, "turnspeed", 9) == 0) 
    { 
        const char* pBeg = &inputString[0];
        char* pEnd;
        int new_speed = strtol(pBeg+10, &pEnd,10);
        if ( new_speed < 10 ) new_speed = 10;
        if ( new_speed > 200 ) new_speed = 200;
        _standardBotDefaultTurnSpeed = new_speed;
        dprint(term,"turn speed set to %d", new_speed);
    }
}

#endif // (CONFIGURED_BOT == STANDARD_BOT)