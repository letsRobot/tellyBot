// EyesAPA102.h
//

#ifndef _EYESAPA102_H_
#define _EYESAPA102_H_

#include "fdserial.h"

void StartEyesAPA102Handler(int clockPin, int dataPin);
void HandleEyeAPA102Commands(const char* inputString, fdserial* term);

#endif // _EYESAPA102_H_
