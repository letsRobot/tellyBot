// StandardBot.h

#ifndef _STANDARDBOT_H_
#define _STANDARDBOT_H_

#include "fdserial.h"

void StartStandardBotController();
void HandleStandardBotCommands(const char* inputString, fdserial *term);

#endif // _STANDARDBOT_H_