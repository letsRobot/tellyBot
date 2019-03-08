// AccessoryServos.c
//

#include "simpletools.h"
#include "AccessoryServos.h"
#include "servo.h"

volatile ConfigInfo* s_currentConfig = 0;
uint8_t* s_flagStates = 0;
volatile uint16_t* s_servoPositions = 0;

volatile int updateServos = 0;

void moveAccessoryServos()
{
    while(1)
    {
        if ( updateServos > 0 )
        {
            for (int i = 0; i < s_currentConfig->numServoInfos; i++)
            {
                servo_angle(s_currentConfig->servoInfos[i].servoPin, s_servoPositions[i]);
            }
            updateServos--;
        }
        else
        {
            pause(10);
        }
    }
}

void StartAccessoryServosController(ConfigInfo* configInfo)
{
    s_currentConfig = configInfo;
    if (s_currentConfig != 0)
    {
        if (s_currentConfig->numFlagInfos > 0)
        {
            s_flagStates = malloc(sizeof(uint8_t) * s_currentConfig->numFlagInfos);
            for (int i = 0; i < s_currentConfig->numFlagInfos; i++)
            {
                s_flagStates[i] = s_currentConfig->flagInfos[i].defaultState;
            }
        }
        if (s_currentConfig->numServoInfos > 0)
        {
            s_servoPositions = malloc(sizeof(uint16_t) * s_currentConfig->numServoInfos);
            for (int i = 0; i < s_currentConfig->numServoInfos; i++)
            {
                s_servoPositions[i] = s_currentConfig->servoInfos[i].defaultPos;
                servo_angle(s_currentConfig->servoInfos[i].servoPin, s_servoPositions[i]);
            }
        }
        
        cog_run(moveAccessoryServos, 128);
    }
}

void HandleAccessoryServosCommands(const char* inputString, fdserial *term)
{
    if ( s_currentConfig != 0 )
    {
        int servosNeedUpdate = 0;
        // loop over all command infos to check for a match (there can be multiples)
        for (int i = 0; i < s_currentConfig->numCommandInfos; i++)
        {
            if (strcmp(inputString, s_currentConfig->commandInfos[i].commandString) == 0)
            {
                // is it a servo command?
                if (s_currentConfig->commandInfos[i].commandType <= LastServoCommand)
                {
                    int servoIndex = s_currentConfig->commandInfos[i].index;
                    ServoInfo* currentServo = &s_currentConfig->servoInfos[servoIndex];
                    // check for flag enabling servo
                    if (currentServo->flagIndex == -1 || s_flagStates[currentServo->flagIndex] != 0)
                    {
                        switch (s_currentConfig->commandInfos[i].commandType)
                        {
                            case IncStep:
                                dprint(term, "IncStep %d : %d -> ", currentServo->servoPin, s_servoPositions[servoIndex] );
                                s_servoPositions[servoIndex] += currentServo->stepSize;
                                if (s_servoPositions[servoIndex] > currentServo->maxPos)
                                {
                                    s_servoPositions[servoIndex] = currentServo->maxPos;
                                }
                                dprint(term, "%d", s_servoPositions[servoIndex] );
                                break;
                            case DecStep:
                                dprint(term, "DecStep %d : %d -> ", currentServo->servoPin, s_servoPositions[servoIndex] );
                                s_servoPositions[servoIndex] -= currentServo->stepSize;
                                if (s_servoPositions[servoIndex] < currentServo->minPos)
                                {
                                    s_servoPositions[servoIndex] = currentServo->minPos;
                                }
                                dprint(term, "%d", s_servoPositions[servoIndex] );
                                break;
                            case GotoDefault:
                                s_servoPositions[servoIndex] = currentServo->defaultPos;
                                dprint(term, "GotoDefault %d : %d", currentServo->servoPin, s_servoPositions[servoIndex]);
                                break;
                            case GotoMin:
                                s_servoPositions[servoIndex] = currentServo->minPos;
                                dprint(term, "GotoMin %d : %d", currentServo->servoPin, s_servoPositions[servoIndex]);
                                break;
                            case GotoMax:
                                s_servoPositions[servoIndex] = currentServo->maxPos;
                                dprint(term, "GotoMax %d : %d", currentServo->servoPin, s_servoPositions[servoIndex]);
                                break;
                        }
                        servosNeedUpdate = 1;
                    }
                }
                else // no, it's a flag command
                {
                    uint8_t flagIndex = s_currentConfig->commandInfos[i].index;
                    FlagInfo* currentFlag = &s_currentConfig->flagInfos[flagIndex];
                    
                    switch (s_currentConfig->commandInfos[i].commandType)
                    {
                        case ToggleFlag:
                            s_flagStates[flagIndex] = !s_flagStates[flagIndex];
                            break;
                        case SetFlag:
                            s_flagStates[flagIndex] = 1;
                            break;
                        case ClearFlag:
                            s_flagStates[flagIndex] = 0;
                            break;
                    }

                    dprint(term, "%s %s", currentFlag->flagName, s_flagStates[flagIndex] ? "on" : "off");
                }
            }
        }
        if (servosNeedUpdate != 0)
        {
            updateServos++;
        }            
    }
}
