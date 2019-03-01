// EyesAPA102.c

#include "BotConfig.h"

#if (CONFIGURED_EYES == EYES_APA102)

// this code assumes two eyes using the 8x8 lumini displays (64 leds per) for now

// emoji stuff uses a lot of memory right now, if you enable it, you will likely have odd issues because of running out of memory 
// particularly with standard_bot which uses a lot of memory too (because of using abdrive360.h).
//#define ENABLE_EMOJI

#include "simpletools.h"
#include "fdserial.h"
#include "apa102.h"
#ifdef ENABLE_EMOJI
#include "emoji.h"
#endif

volatile int update_eyes_apa102 = 0;
volatile int eye_command_apa102 = 0;
volatile uint8_t current_mask_apa102 = 1;
volatile uint8_t update_mask_apa102 = 0;
volatile uint8_t brightness_apa102 = 1;
volatile uint32_t mask_color_apa102 = COLOR_GRAY;
volatile uint32_t mask_bg_color_apa102 = COLOR_BLACK;
volatile uint8_t pixel_apa102 = 0;
volatile uint32_t pixel_color_apa102 = 0xFFFFFF;
volatile uint32_t eye_color_apa102 = 0xFFFFFF;
#ifdef ENABLE_EMOJI
volatile uint8_t eyes_emoji = 0;
#endif

#define EYES_BLINK 1
#define EYES_INCREASE_BRIGHTNESS 2
#define EYES_DECREASE_BRIGHTNESS 3
#define EYES_SET_COLOR_SINGLE_PIXEL 4
#define EYES_SET_COLOR_ALL_PIXELS 5
#define EYES_SET_MASK 6
#define EYES_DO_COP 7
#ifdef ENABLE_EMOJI
#define EYES_EMOJI 8
#endif

/* Helper macros */
#define HEX__(n) 0x##n
#define B8__(x) ((x&0x0000000F)?1:0) \
+((x&0x000000F0)?2:0) \
+((x&0x00000F00)?4:0) \
+((x&0x0000F000)?8:0) \
+((x&0x000F0000)?16:0) \
+((x&0x00F00000)?32:0) \
+((x&0x0F000000)?64:0) \
+((x&0xF0000000)?128:0)

/* User macros */
#define B8(d) ((unsigned char)B8__(HEX__(d)))

// masks
typedef struct
{
    uint8_t index;
    uint8_t rotation; // 0 = none, 1 = flip x, 2 = flip y, 3 = flip both x and y
    uint8_t index2;
    uint8_t rotation2;    
} maskInfo;

maskInfo maskData[11] =
{
    { 0, 0, 0, 0 }, // all on
    { 1, 0, 1, 0 }, // o.o
    { 2, 0, 2, 0 }, // -.-
    { 3, 0, 3, 0 }, // v.v
    { 3, 2, 3, 2 }, // ^.^
    { 4, 0, 4, 0 }, // x.x
    { 5, 0, 5, 2 }, // \./
    { 6, 0, 6, 0 }, // T.T
    { 7, 0, 7, 0 }, // u.u
    { 7, 2, 7, 2 }, // n.n
    { 8, 0, 8, 0 }  // smiley's
};    
    
uint8_t masks[9][8] =
{
    {
        B8(11111111),
        B8(11111111),
        B8(11111111),
        B8(11111111),
        B8(11111111),
        B8(11111111),
        B8(11111111),
        B8(11111111)
    },
    {
        B8(00000000),
        B8(00111100),
        B8(01111110),
        B8(01100110),
        B8(01100110),
        B8(01111110),
        B8(00111100),
        B8(00000000)
    },
    {
        B8(00000000),
        B8(00000000),
        B8(00000000),
        B8(01111110),
        B8(01111110),
        B8(00000000),
        B8(00000000),
        B8(00000000)
    },
    {
        B8(00000000),
        B8(00000000),
        B8(01000010),
        B8(01100110),
        B8(01111110),
        B8(00111100),
        B8(00011000),
        B8(00000000)
    },
    {
        B8(10000001),
        B8(11000011),
        B8(01100110),
        B8(00111100),
        B8(00111100),
        B8(01100110),
        B8(11000011),
        B8(10000001)
    },
    {
        B8(01000000),
        B8(01100000),
        B8(01110000),
        B8(00111000),
        B8(00011100),
        B8(00001110),
        B8(00000110),
        B8(00000010)
    },
    {
        B8(00000000),
        B8(01111110),
        B8(01111110),
        B8(00011000),
        B8(00011000),
        B8(00011000),
        B8(00011000),
        B8(00000000)
    },
    {
        B8(00000000),
        B8(01100110),
        B8(01100110),
        B8(01100110),
        B8(01100110),
        B8(01111110),
        B8(00111100),
        B8(00000000)
    },
    {
        B8(00111100),
        B8(01000010),
        B8(10100101),
        B8(10000001),
        B8(10100101),
        B8(10011001),
        B8(01000010),
        B8(00111100)
    }
};        


#define NUM_LEDS 128
uint32_t LEDs[NUM_LEDS];

uint32_t wheel(int pos)
{
    uint32_t color;

// Creates color from 0 to 255 position input
// -- colors transition r-g-b back to r

    // red range
    if (pos < 85)
    {
        color = COLOR(255-pos*3, pos*3, 0);
    }    
    // green range
    else if (pos < 170)
    {
        pos -= 85;
        color = COLOR(0, 255-pos*3, pos*3);
    }
    // blue range
    else
    {
        pos -= 170;
        color = COLOR(pos*3, 0, 255-pos*3);
    }
    
    return color;
}

#ifdef ENABLE_EMOJI
// emoji image is 80x80 pixels, 100 emoji in a 10x10 grid
// whichEye = 0 for first, 1 for second, 2 for both
void emojiDraw(int whichEmoji, int whichEye)
{
    int offsetX = (whichEmoji % 10) * 8;
    int offsetY = (whichEmoji / 10) * 8;
    uint32_t upper8bits = (0xE0 + brightness_apa102) << 24;
    
    for (int y = 0; y < 8; y++ )
    {
        for (int x = 0; x < 8; x++ )
        {
            int index = ((offsetY + y) * 80) + offsetX + x;
            int pixel = emoji_bitmap[index];
            uint32_t color = upper8bits | COLORX(emoji_palette[pixel].r, emoji_palette[pixel].g, emoji_palette[pixel].b, 20 );
            int ledIndex = ( ( 7 - y ) * 8 ) + x;
            if ( whichEye == 0 || whichEye == 2 )
            {
                LEDs[ledIndex] = color;
            }
            if ( whichEye == 1 || whichEye == 2 )
            {
                LEDs[64 + ledIndex] = color;
            }
        }
    }                            
    SendLeds( LEDs, NUM_LEDS );
}
#endif

void maskDraw(maskInfo maskData, uint32_t color, uint32_t bgColor)
{
    uint32_t upper8bits = (0xE0 + brightness_apa102) << 24;
    uint8_t* mask = masks[maskData.index];
    uint8_t* mask2 = masks[maskData.index2];
    
    for ( int i = 0; i < 8; i++ )
    {
        for ( int j = 0; j < 8; j++ )
        {
            int x = ( ( 7 - i ) * 8 ) + j;
            
            // first eye
            int ii = ( maskData.rotation == 0 || maskData.rotation == 1 ) ? i : 7 - i;
            int jj = ( maskData.rotation == 0 || maskData.rotation == 2 ) ? j : 7 - j;
            if ( ( mask[ii] >> ( 7 - jj ) ) & 1 )
            {
                LEDs[ x ] = upper8bits | color;
            }
            else
            {
                LEDs[ x ] = upper8bits | bgColor;
            }
            
            // second eye
            ii = ( maskData.rotation2 == 0 || maskData.rotation2 == 1 ) ? i : 7 - i;
            jj = ( maskData.rotation2 == 0 || maskData.rotation2 == 2 ) ? j : 7 - j;
            if ( ( mask2[ii] >> ( 7 - jj ) ) & 1 )
            {
                LEDs[ 64 + x ] = upper8bits | color;
            }
            else
            {
                LEDs[ 64 + x ] = upper8bits | bgColor;
            }
        }
    }

    SendLeds( LEDs, NUM_LEDS );
}    

void setAll( uint32_t color )
{
    uint32_t upper8bits = (0xE0 + brightness_apa102) << 24;
    
    for ( int i = 0; i < NUM_LEDS; i++ )
    {
        LEDs[i] = upper8bits | color;
    }
    SendLeds( LEDs, NUM_LEDS );
}

void setSingle( uint8_t led, uint32_t color )
{
    uint32_t upper8bits = (0xE0 + brightness_apa102) << 24;
    LEDs[led] = upper8bits | color;
    SendLeds( LEDs, NUM_LEDS );
}

void eyes_blink_apa102()
{
    maskDraw(maskData[current_mask_apa102], mask_color_apa102, mask_bg_color_apa102);
    pause(500);
    maskDraw(maskData[2], mask_color_apa102, mask_bg_color_apa102);
    pause(500);
    maskDraw(maskData[current_mask_apa102], mask_color_apa102, mask_bg_color_apa102);
    pause(1);
}

void eyes_cop_apa102()
{
    uint32_t upper8bits = (0xE0 + brightness_apa102) << 24;
    for (int a=0; a <= 5; ++a)
    {
        for ( int i = 0; i < NUM_LEDS/2; i++ )
        {
            LEDs[i] = upper8bits | 0xff0000;
        }
        for ( int i = NUM_LEDS/2; i < NUM_LEDS; i++ )
        {
            LEDs[i] = upper8bits | 0x0000ff;
        }
        SendLeds( LEDs, NUM_LEDS );
        pause(400);
        for ( int i = 0; i < NUM_LEDS/2; i++ )
        {
            LEDs[i] = upper8bits | 0x0000ff;
        }
        for ( int i = NUM_LEDS/2; i < NUM_LEDS; i++ )
        {
            LEDs[i] = upper8bits | 0xff0000;
        }
        SendLeds( LEDs, NUM_LEDS );
        pause(400);
    }
    maskDraw(maskData[current_mask_apa102], mask_color_apa102, mask_bg_color_apa102);
}

volatile int _apa102ClkPin = 0;
volatile int _apa102DataPin = 0;
void handle_eyes_apa102()
{
    InitApa102(_apa102ClkPin, _apa102DataPin);
    setAll(0);
    
    while (1)
    {
        if (update_eyes_apa102 > 0)
        {
            // I'm doing this local copy of the update_eyes variable so that I can clear update_eyes variable right away
            // this makes it so that if you get another eye command while it's still in the middle of doing the last one
            // it will do that new one after.  This only allows one queued up command, but that should be fine for most cases
            eye_command_apa102 = update_eyes_apa102;
            update_eyes_apa102 = 0;
            switch(eye_command_apa102)
            {
                case EYES_BLINK:
                    eyes_blink_apa102();
                    break;
                case EYES_INCREASE_BRIGHTNESS:
                    brightness_apa102++;
                    if ( brightness_apa102 > 31 )
                    {
                        brightness_apa102 = 31;
                    }                        
                    maskDraw(maskData[current_mask_apa102], mask_color_apa102, mask_bg_color_apa102);
                    break;
                case EYES_DECREASE_BRIGHTNESS:
                    brightness_apa102--;
                    if ( brightness_apa102 < 1 )
                    {
                        brightness_apa102 = 1;
                    }                        
                    maskDraw(maskData[current_mask_apa102], mask_color_apa102, mask_bg_color_apa102);
                    break;
                case EYES_SET_COLOR_SINGLE_PIXEL:
                    setSingle(pixel_apa102, pixel_color_apa102);
                    break;
                case EYES_SET_COLOR_ALL_PIXELS:
                    setAll(pixel_color_apa102);
                    break;
                case EYES_SET_MASK:
                    current_mask_apa102 = update_mask_apa102;
                    maskDraw(maskData[current_mask_apa102], mask_color_apa102, mask_bg_color_apa102);
                    break;
			 case EYES_DO_COP:
                    eyes_cop_apa102();
                    break;
#ifdef ENABLE_EMOJI
                case EYES_EMOJI:
                    emojiDraw(eyes_emoji, 2);
                    break;
#endif
            }
        }
    }
}

void StartEyesAPA102Handler(int clockPin, int dataPin)
{
    _apa102ClkPin = clockPin;
    _apa102DataPin = dataPin;
    
    pause(250);
    cog_run(handle_eyes_apa102, 128);
    
    pause(250);
    update_eyes_apa102 = EYES_BLINK;   
}
    
void HandleEyeAPA102Commands(const char* inputString, fdserial* term)
{
    if (strcmp(inputString, "brightness_up") == 0)
    {
        update_eyes_apa102 = EYES_INCREASE_BRIGHTNESS;
        dprint(term,"brightness increased");
    }

    if (strcmp(inputString, "brightness_down") == 0)
    {
        update_eyes_apa102 = EYES_DECREASE_BRIGHTNESS;
        dprint(term,"brightness decreased");
    }

    if (strcmp(inputString, "ms0") == 0)
    {
        dprint(term,"mask 0");
        update_mask_apa102 = 0;
        update_eyes_apa102 = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms1") == 0)
    {
        dprint(term,"mask 1");
        update_mask_apa102 = 1;
        update_eyes_apa102 = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms2") == 0)
    {
        dprint(term,"mask 2");
        update_mask_apa102 = 2;
        update_eyes_apa102 = EYES_SET_MASK;
    }

    if (strcmp(inputString, "ms3") == 0)
    {
        dprint(term,"mask 3");
        update_mask_apa102 = 3;
        update_eyes_apa102 = EYES_SET_MASK;
    }
    if (strcmp(inputString, "ms4") == 0)
    {
        dprint(term,"mask 4");
        update_mask_apa102 = 4;
        update_eyes_apa102 = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms5") == 0)
    {
        dprint(term,"mask 5");
        update_mask_apa102 = 5;
        update_eyes_apa102 = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms6") == 0)
    {
        dprint(term,"mask 6");
        update_mask_apa102 = 6;
        mask_color_apa102 = COLOR_RED;
        update_eyes_apa102 = EYES_SET_MASK;
    }

    if (strcmp(inputString, "ms7") == 0)
    {
        dprint(term,"mask 7");
        update_mask_apa102 = 7;
        update_eyes_apa102 = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms8") == 0)
    {
        dprint(term,"mask 8");
        update_mask_apa102 = 8;
        update_eyes_apa102 = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms9") == 0)
    {
        dprint(term,"mask 9");
        update_mask_apa102 = 9;
        update_eyes_apa102 = EYES_SET_MASK;
    }

    if (strcmp(inputString, "ms10") == 0)
    {
        dprint(term,"mask 10");
        update_mask_apa102 = 10;
        update_eyes_apa102 = EYES_SET_MASK;
    }

    if (strcmp(inputString, "blink") == 0)
    {
        dprint(term,"blink");
        update_eyes_apa102 = EYES_BLINK;
    }
    if (strcmp(inputString, "cop") == 0)
    {
        dprint(term,"cop");
        update_eyes_apa102 = EYES_DO_COP;
    }
    if (strncmp(inputString, "mfg",3) == 0) 
    { 
        const char* pBeg = &inputString[0];
        char* pEnd;
        mask_color_apa102 = strtol(pBeg+4, &pEnd,16);
        mask_color_apa102 = RGBTOBGR(mask_color_apa102);
        if (mask_color_apa102 > 0xFFFFFF)
        {
            mask_color_apa102 = 0xFFFFFF;
        }
        dprint(term,"%d",mask_color_apa102);
        update_mask_apa102 = current_mask_apa102;
        update_eyes_apa102 = EYES_SET_MASK;
    }                   

    if (strncmp(inputString, "mbg",3) == 0) 
    { 
        const char* pBeg = &inputString[0];
        char* pEnd;
        mask_bg_color_apa102 = strtol(pBeg+4, &pEnd,16);
        mask_bg_color_apa102 = RGBTOBGR(mask_bg_color_apa102);
        if (mask_bg_color_apa102 > 0xFFFFFF)
        {
            mask_bg_color_apa102 = 0xFFFFFF;
        }
        dprint(term,"%d",mask_bg_color_apa102);
        update_mask_apa102 = current_mask_apa102;
        update_eyes_apa102 = EYES_SET_MASK;
    }                   

    if (strncmp(inputString, "led",3) == 0) 
    { 
        const char* pBeg = &inputString[0];
        char* pEnd;
        pixel_apa102 = strtol(pBeg+4, &pEnd,10);
        pixel_color_apa102 = strtol(pEnd, &pEnd,16);
        pixel_color_apa102 = RGBTOBGR(pixel_color_apa102);
        dprint(term,"%d",pixel_color_apa102);
        if ((pixel_apa102 < NUM_LEDS)&&(pixel_color_apa102<=0xFFFFFF))
        {
            update_eyes_apa102 = EYES_SET_COLOR_SINGLE_PIXEL;
        }
    }           

    if (strncmp(inputString, "leds",4) == 0) 
    { 
        const char* pBeg = &inputString[0];
        char* pEnd;
        pixel_color_apa102 = strtol(pBeg+5, &pEnd,16);
        pixel_color_apa102 = RGBTOBGR(pixel_color_apa102);
        dprint(term,"%d",pixel_color_apa102);
        if (pixel_color_apa102<=0xFFFFFF)
        {
            update_eyes_apa102 = EYES_SET_COLOR_ALL_PIXELS;
        }
    }

#ifdef ENABLE_EMOJI
    if (strncmp(inputString, "emoji",5) == 0) 
    { 
        const char* pBeg = &inputString[0];
        char* pEnd;
        eyes_emoji = strtol(pBeg+6, &pEnd,10);
        dprint(term,"emoji %d",eyes_emoji);
        if ( eyes_emoji < 100 )
        {
            update_eyes_apa102 = EYES_EMOJI;
        }
    }
#endif    
}

#endif // (CONFIGURED_EYES == EYES_APA102)