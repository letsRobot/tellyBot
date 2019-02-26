// ParallaxBot.c
//

#include "BotConfig.h"

#include "simpletools.h"
#include "fdserial.h"

#if (CONFIGURED_BOT == TRI_BOT)
#include "TriBot.h"
#else
#include "StandardBot.h"
#endif

#if (CONFIGURED_EYES == EYES_WS2812)
#include "EyesWS2812.h"
#else
#include "EyesAPA102.h"
#endif

fdserial *term; //enables full-duplex serial communication of the terminal (In other words, 2 way signals between the computer and the robot)

void StartBotController(int type)
{
#if (CONFIGURED_BOT == TRI_BOT)
    StartTriBotController();
#else
    StartStandardBotController();
#endif
}

void HandleBotCommands(const char* inputString, fdserial *term)
{
#if (CONFIGURED_BOT == TRI_BOT)
    HandleTriBotCommands(inputString, term);
#else
    HandleStandardBotCommands(inputString, term);
#endif
}

void StartEyesHandler(int type)
{
#if (CONFIGURED_EYES == EYES_WS2812)
    StartEyesWS2812Handler( LED_DATA_PIN );
#else
    StartEyesAPA102Handler( LED_CLOCK_PIN, LED_DATA_PIN );
#endif
}    

void HandleEyeCommands(const char* inputString, fdserial *term)
{
#if (CONFIGURED_EYES == EYES_WS2812)
    HandleEyeWS2812Commands(inputString, term);
#else    
    HandleEyeAPA102Commands(inputString, term);
#endif
}

int main()
{
    // Shutdown the default simpleterm
    simpleterm_close();
    // and start up the full-duplex serial driver for the terminal
    term = fdserial_open(31, 30, 0, SERIAL_BITRATE);

    StartBotController(CONFIGURED_BOT);
    StartEyesHandler(CONFIGURED_EYES);

    char c;  
    int inputStringLength = 64;
    char inputString[inputStringLength];
    int sPos = 0;

    while (1)
    {
        if (fdserial_rxReady(term)!=0)
        {
            c = fdserial_rxChar(term); //Get the character entered from the terminal

            if (c != -1)
            {
                if ((int)c == 13 || (int)c == 10)
                {
                    dprint(term, "\nreceived line:");
                    dprint(term, inputString);
                    dprint(term, "\n");
                    
                    HandleBotCommands(inputString, term);
                    HandleEyeCommands(inputString, term);
                    dprint(term, "\n");
                    
                    sPos = 0;
                    inputString[0] = 0; // clear string
                }
                else if (sPos < inputStringLength - 1)
                {
                    // record next character
                    inputString[sPos] = c;
                    sPos += 1;
                    inputString[sPos] = 0; // make sure last element of string is 0
                    dprint(term, "%c", c);
                }  
            }            
        }      
    }   
}
