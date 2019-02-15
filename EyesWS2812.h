// Eyes.h
//

#ifndef _EYES_H_
#define _EYES_H_

#include "fdserial.h"

void StartEyesHandler(int pin);
void HandleEyeCommands(const char* inputString, fdserial* term);

#endif // _EYES_H_