// EyesWS2812.h
//

#ifndef _EYESWS2812_H_
#define _EYESWS2812_H_

#include "fdserial.h"

void StartEyesWS2812Handler(int pin);
void HandleEyeWS2812Commands(const char* inputString, fdserial* term);

#endif // _EYESWS2812_H_