// TriBot.h
//

#ifndef _TRIBOT_H_
#define _TRIBOT_H_

#include "fdserial.h"

void StartTriBotController();
void HandleTriBotCommands(const char* inputString, fdserial *term);

#endif // _TRIBOT_H_