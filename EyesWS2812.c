// EyesWS2812.c
//
#include "BotConfig.h"

#if (CONFIGURED_EYES == EYES_WS2812)

#include "simpletools.h"
#include "fdserial.h"
#include "ws2812.h"

#define LED_COUNT   18

ws2812 *led_driver;
int _eyesLedPin;
  
// used by handle_eyes function
volatile int update_eyes = 0;
volatile uint8_t pixel = 0;
volatile uint32_t pixel_color = 0xFFFFFF;
volatile uint8_t brightness = 10;
volatile uint32_t eye_color = 0xFFFFFF;
volatile int eye_command = 0;
volatile uint8_t current_mask = 1;
volatile uint8_t update_mask = 0;

uint32_t ledColors[LED_COUNT];
uint32_t dim_array[LED_COUNT];

#define EYES_BLINK 1
#define EYES_INCREASE_BRIGHTNESS 2
#define EYES_DECREASE_BRIGHTNESS 3
#define EYES_SET_COLOR_SINGLE_PIXEL 4
#define EYES_SET_COLOR_ALL_PIXELS 5
#define EYES_SET_MASK 6
#define EYES_DO_LRN 7
#define EYES_DO_COP 8

char nmask[10][LED_COUNT] = 
{
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // all on
    {1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1}, // o.o
    {0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,0,0,0}, // -.-
    {0,1,0,1,0,1,0,0,0,0,1,0,1,0,1,0,0,0}, // v.v
    {0,0,0,1,0,1,0,1,0,0,0,0,1,0,1,0,1,0}, // ^.^
    {1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1}, // x.x
    {0,1,1,1,1,0,1,0,0,1,1,0,0,1,1,0,0,1}, // \./
    {0,1,0,0,1,0,1,1,1,0,1,0,0,1,0,1,1,1}, // T.T
    {1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,0,1}, // U.U
    {1,0,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1}, // n.n
};

uint32_t cop1[LED_COUNT] = {0xFF0000 ,0xFF0000 ,0xFF0000 ,0xFF0000 ,0xFF0000 ,0xFF0000 ,0xFF0000 ,0xFF0000 ,0xFF0000 ,0x0000FF ,0x0000FF ,0x0000FF ,0x0000FF ,0x0000FF ,0x0000FF ,0x0000FF ,0x0000FF ,0x0000FF};
uint32_t cop2[LED_COUNT] = {0x0000FF ,0x0000FF ,0x0000FF ,0x0000FF ,0x0000FF ,0x0000FF ,0x0000FF ,0x0000FF ,0x0000FF ,0xFF0000 ,0xFF0000 ,0xFF0000 ,0xFF0000 ,0xFF0000 ,0xFF0000 ,0xFF0000 ,0xFF0000 ,0xFF0000};
uint32_t lrn[LED_COUNT] = {0xA652AF ,0x000000 ,0x5F79FF ,0xF16B74 ,0x000000 ,0x21BCE5 ,0xF9AA67 ,0xF3EB48 ,0x97E062 ,0xA652AF ,0x000000 ,0x5F79FF ,0xF16B74 ,0x000000 ,0x21BCE5 ,0xF9AA67 ,0xF3EB48 ,0x97E062};

void refresh_eyes()
{
    for (int i2 = 0; i2 < LED_COUNT; ++i2)
    {
        uint32_t red = (ledColors[i2]>>16) & 0xFF;
        red = red*brightness/255;
        uint32_t green = (ledColors[i2]>>8) & 0xFF;
        green = green*brightness/255;
        uint32_t blue = (ledColors[i2]) & 0xFF;
        blue = blue*brightness/255;
        uint32_t scaled_color = (red << 16)+(green << 8)+(blue);
        dim_array[i2] = scaled_color;      
    }   

    ws2812_set(led_driver, _eyesLedPin, dim_array, LED_COUNT);
}

void set_neopixel(uint8_t pixel_num, uint32_t color)
{
    if(pixel_num <LED_COUNT)
    {
        ledColors[pixel_num] = color;
    }
    refresh_eyes();
}

void set_neopixel_group(uint32_t color)
{
    for (int i = 0; i < LED_COUNT; ++i)
    {
        ledColors[i] = color;
    }   
    refresh_eyes();
}

void refresh_eyes_masked(char ledmask[])
{
    for (int i2 = 0; i2 < LED_COUNT; ++i2)
    {
        if (ledmask[i2] == 1)
        {
            uint32_t red = (ledColors[i2]>>16) & 0xFF;
            red = red*brightness/255;
            uint32_t green = (ledColors[i2]>>8) & 0xFF;
            green = green*brightness/255;
            uint32_t blue = (ledColors[i2]) & 0xFF;
            blue = blue*brightness/255;
            uint32_t scaled_color = (red << 16)+(green << 8)+(blue);
            dim_array[i2] = scaled_color;
        }
        else
        {
            dim_array[i2] = 0x000000;
        }
    }
    ws2812_set(led_driver, _eyesLedPin, dim_array, LED_COUNT);
}

void increase_brightness()
{
    int16_t temp_brightness = brightness+10;
    if(temp_brightness>255)
    {
        temp_brightness = 255;
    } 
    brightness = temp_brightness;
    refresh_eyes_masked(nmask[current_mask]);
}  

void decrease_brightness()
{
    int16_t temp_brightness = brightness-10;
    if(temp_brightness<=1)
    {
        temp_brightness = 2;
    } 
    brightness = temp_brightness;
    refresh_eyes_masked(nmask[current_mask]);
} 

void eyes_blink()
{
    refresh_eyes_masked(nmask[current_mask == 2 ? 1 : current_mask]);
    pause(500);
    refresh_eyes_masked(nmask[2]);
    pause(500);
    refresh_eyes_masked(nmask[current_mask == 2 ? 1 : current_mask]);
    pause(1);
}

void eyes_cop()
{
    for (int a=0; a <= 5; ++a)
    {
        memcpy(ledColors, cop1, sizeof(cop1));
        refresh_eyes_masked(nmask[0]);
        pause(400);
        memcpy(ledColors, cop2, sizeof(cop2));
        refresh_eyes_masked(nmask[0]);
        pause(400);
    }
    set_neopixel_group(0xFFFFFF);
    refresh_eyes_masked(nmask[current_mask]);
}

void handle_eyes()
{
    set_neopixel_group(0xffffff);
    
    while (1)
    {
        if (update_eyes > 0)
        {
            // I'm doing this local copy of the update_eyes variable so that I can clear update_eyes variable right away
            // this makes it so that if you get another eye command while it's still in the middle of doing the last one
            // it will do that new one after.  This only allows one queued up command, but that should be fine for most cases
            eye_command = update_eyes;
            update_eyes = 0;
            switch(eye_command)
            {
                case EYES_BLINK:
                    eyes_blink();
                    break;
                case EYES_INCREASE_BRIGHTNESS:
                    increase_brightness();
                    break;
                case EYES_DECREASE_BRIGHTNESS:
                    decrease_brightness();
                    break;
                case EYES_SET_COLOR_SINGLE_PIXEL:
                    set_neopixel(pixel, pixel_color);
                    break;
                case EYES_SET_COLOR_ALL_PIXELS:
                    set_neopixel_group(pixel_color);
                    break;
                case EYES_SET_MASK:
                    current_mask = update_mask;
                    refresh_eyes_masked(nmask[current_mask]);
                    break;
                case EYES_DO_LRN:
                    memcpy(ledColors, lrn, sizeof(lrn));
                    current_mask = update_mask;
                    refresh_eyes_masked(nmask[current_mask]);
                    break;
                case EYES_DO_COP:
                    eyes_cop();
                    break;
            }
        }
    }
}

void StartEyesWS2812Handler(int pin)
{
    _eyesLedPin = pin;

    // load the LED driver
    if (!(led_driver = ws2812b_open()))
    {
        return;
    }

    pause(250);
    cog_run(handle_eyes, 128);

    pause(100);
    update_eyes = EYES_BLINK;   
}    

void HandleEyeWS2812Commands(const char* inputString, fdserial* term)
{
    if (strcmp(inputString, "brightness_up") == 0)
    {
        update_eyes = EYES_INCREASE_BRIGHTNESS;
        dprint(term,"brightness increased");
    }

    if (strcmp(inputString, "brightness_down") == 0)
    {
        update_eyes = EYES_DECREASE_BRIGHTNESS;
        dprint(term,"brightness decreased");
    }

    if (strcmp(inputString, "ms0") == 0)
    {
        dprint(term,"mask 0");
        update_mask = 0;
        update_eyes = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms1") == 0)
    {
        dprint(term,"mask 1");
        update_mask = 1;
        update_eyes = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms2") == 0)
    {
        dprint(term,"mask 2");
        update_mask = 2;
        update_eyes = EYES_SET_MASK;
    }
      
    if (strcmp(inputString, "ms3") == 0)
    {
        dprint(term,"mask 3");
        update_mask = 3;
        update_eyes = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms4") == 0)
    {
        dprint(term,"mask 4");
        update_mask = 4;
        update_eyes = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms5") == 0)
    {
        dprint(term,"mask 5");
        update_mask = 5;
        update_eyes = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms6") == 0)
    {
        dprint(term,"mask 6");
        update_mask = 6;
        update_eyes = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms7") == 0)
    {
        dprint(term,"mask 7");
        update_mask = 7;
        update_eyes = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms8") == 0)
    {
        dprint(term,"mask 8");
        update_mask = 8;
        update_eyes = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "ms9") == 0)
    {
        dprint(term,"mask 9");
        update_mask = 9;
        update_eyes = EYES_SET_MASK;
    }
    
    if (strcmp(inputString, "lrn") == 0)
    {
        dprint(term,"mask 9 lrn");
        update_mask = 9;
        update_eyes = EYES_DO_LRN;
        refresh_eyes();
    }
    
    if (strcmp(inputString, "cop") == 0)
    {
        dprint(term,"cop");
        update_eyes = EYES_DO_COP;
    }

    if (strcmp(inputString, "blink") == 0)
    {
        dprint(term,"blink");
        update_eyes = EYES_BLINK;
    }

    if (strncmp(inputString, "led",3) == 0) 
    { 
        const char* pBeg = &inputString[0];
        char* pEnd;
        pixel = strtol(pBeg+4, &pEnd,10);
        pixel_color = strtol(pEnd, &pEnd,16);
        dprint(term,"%d",pixel_color);
        if ((pixel < LED_COUNT)&&(pixel_color<=0xFFFFFF))
        {
            update_eyes = EYES_SET_COLOR_SINGLE_PIXEL;
        }
    }           

    if (strncmp(inputString, "leds",4) == 0) 
    { 
        const char* pBeg = &inputString[0];
        char* pEnd;
        pixel_color = strtol(pBeg+5, &pEnd,16);
        dprint(term,"%d",pixel_color);
        if (pixel_color<=0xFFFFFF)
        {
            update_eyes = EYES_SET_COLOR_ALL_PIXELS;
        }
    }                   
}

#endif // (CONFIGURED_EYES == EYES_WS2812)