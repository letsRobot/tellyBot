// TiltGripperAccessoryServosConfig.h
//

#include "AccessoryServos.h"

ServoInfo dartServos[2] =
{
    // gripper
    {  0,  100,  100, 1900, 100, 0 },
    
    // tilt
    { 17, 1000,  100, 1900, 100, -1 }
    
};    

FlagInfo dartFlags[1] =
{
    { "gripper", 1 }
};

CommandInfo dartCommands[10] =
{
    // gripper
    {"go", IncStep, 0},
    {"gc", DecStep, 0},
    {"gfo", GotoMin, 0},
    {"gfc", GotoMax, 0},
    
    // tilt
    {"tu", IncStep, 1},
    {"td", DecStep, 1},
    {"tmin", GotoMin, 1},
    {"tmax", GotoMax, 1},
    {"tc", GotoDefault, 1},
    
    // flags
    {"gripper", ToggleFlag, 0}
};

ConfigInfo AccessoryServosConfig = 
{
    dartServos,
    2,
    dartCommands,
    10,
    dartFlags,
    1
}; 
