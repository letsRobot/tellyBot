// AccessoryServos.h
//

#ifndef _ACCESSORYSERVOS_H_
#define _ACCESSORYSERVOS_H_

#include "fdserial.h"

typedef struct
{
    uint8_t servoPin;
    uint16_t defaultPos;
    uint16_t minPos;
    uint16_t maxPos;
    uint16_t stepSize;
    int8_t flagIndex;  // -1 means no flag, which flag disables this servo?
} ServoInfo;

typedef enum
{
    IncStep,
    DecStep,
    GotoDefault,
    GotoMin,
    GotoMax,
    LastServoCommand = GotoMax,
    
    ToggleFlag,
    SetFlag,
    ClearFlag
}  CommandType;

typedef struct
{
    char* commandString;
    CommandType commandType;
    int index; // depends on command type (for servo motion commands this will be the servo index, for flag commands it's the flag index)
} CommandInfo;

typedef struct
{
    char*   flagName;
    uint8_t defaultState;
} FlagInfo;    

typedef struct
{
    ServoInfo*      servoInfos;
    int             numServoInfos;
    CommandInfo*    commandInfos;
    int             numCommandInfos;
    FlagInfo*       flagInfos;
    int             numFlagInfos;
} ConfigInfo;    

void StartAccessoryServosController(ConfigInfo* configInfo);
void HandleAccessoryServosCommands(const char* inputString, fdserial *term);

#endif // _ACCESSORYSERVOS_H_