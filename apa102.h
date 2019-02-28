// apa102.h
// Driver for APA102 RGB LEDs (e.g. sparkfun LuMi & LuMini RGB LED boards)
// by Roy Eltham

#ifndef _APA102_H_
#define _APA102_H_

#define COLOR(r, g, b)      (((b) << 16) | ((g) << 8) | (r))
#define SCALE(x, l)         ((x) * (l) / 255)
#define COLORX(r, g, b, l)  ((SCALE(b, l) << 16) | (SCALE(g, l) << 8) | SCALE(r, l))
#define RGBTOBGR(x)         (((x) & 0x000000ff) << 16 | ((x) & 0x0000ff00) | ((x) & 0x00ff0000) >> 16);

//                         BBGGRR
#define COLOR_BLACK      0x000000
#define COLOR_RED        0x0000FF
#define COLOR_GREEN      0x00FF00
#define COLOR_BLUE       0xFF0000
#define COLOR_WHITE      0xFFFFFF
#define COLOR_CYAN       0xFFFF00
#define COLOR_MAGENTA    0xFF00FF
#define COLOR_YELLOW     0x00FFFF
#define COLOR_CHARTREUSE 0x00FF7F
#define COLOR_ORANGE     0x0060FF
#define COLOR_AQUAMARINE 0xD4FF7F
#define COLOR_PINK       0x5F5FFF
#define COLOR_TURQUOISE  0xC0E03F
#define COLOR_REALWHITE  0xFFFFC8
#define COLOR_INDIGO     0x7F003F
#define COLOR_VIOLET     0xBF7FBF
#define COLOR_MAROON     0x100032
#define COLOR_BROWN      0x00060E
#define COLOR_CRIMSON    0x3C28DC
#define COLOR_PURPLE     0xFF008C
#define COLOR_GRAY       0x3F3F3F

void InitApa102(int clkPin, int dataPin);
void SendLeds(const uint32_t* leds, int numLeds);

#endif