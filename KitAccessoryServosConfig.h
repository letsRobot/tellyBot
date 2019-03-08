// KitAccessoryServosConfig.h
//

#include "AccessoryServos.h"

ServoInfo kitServos[8] =
{
    // left arm
    { 10, 1100,   0, 1800, 20, 0 },
    { 11, 1500,   0, 1800, 20, 0 },
    {  9,  900,   0, 1800, 20, 0 },
    
    // right arm
    { 16,  800,   0, 2000, 20, 0 },
    { 18,    0,   0, 1800, 20, 0 },
    { 17, 1000,   0, 1800, 20, 0 },
    
    // pan/tilt
    { 19,  900, 100, 1700, 20, 1 },
    { 20,  900, 100, 1700, 20, 1 }
};    

FlagInfo kitFlags[2] =
{
    { "arms", 1 },
    { "head", 1 }
};

CommandInfo kitCommands[20] =
{
    // left arm
    {"i", IncStep, 0},
    {"h", DecStep, 0},
    {"j", IncStep, 1},
    {"k", DecStep, 1},
    {"m", IncStep, 2},
    {"n", DecStep, 2},

    // right arm
    {"g", IncStep, 3},
    {"o", DecStep, 3},
    {"y", IncStep, 4},
    {"u", DecStep, 4},
    {"v", IncStep, 5},
    {"w", DecStep, 5},
    
    // pan/tilt
    {"pm", IncStep, 6},
    {"pp", DecStep, 6},
    {"tm", IncStep, 7},
    {"tp", DecStep, 7},
    {"center", GotoDefault, 6},
    {"center", GotoDefault, 7},
    
    // flags
    {"arms", ToggleFlag, 0},
    {"head", ToggleFlag, 1}
};

ConfigInfo AccessoryServosConfig = 
{
    kitServos,
    8,
    kitCommands,
    20,
    kitFlags,
    2
}; 
