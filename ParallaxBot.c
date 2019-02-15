// ParallaxBot.c
//

// comment this line out to switch the StandardBot instead of TriBot
#define STANDARD_BOT    0
#define TRI_BOT         1

#define CONFIGURED_BOT  TRI_BOT
#define LED_PIN         21

#include "simpletools.h"
#include "fdserial.h"

#include "TriBot.h"
#include "StandardBot.h"
#include "EyesWS2812.h"

fdserial *term; //enables full-duplex serial communication of the terminal (In other words, 2 way signals between the computer and the robot)
int botType;

void StartBotController(int type)
{
    botType = type;
    if ( botType == STANDARD_BOT )
    {
        StartStandardBotController();
    }
    else if ( botType == TRI_BOT )
    {
        StartTriBotController();
    }
}

void HandleBotCommands(const char* inputString, fdserial *term)
{
    if ( botType == STANDARD_BOT )
    {
        HandleStandardBotCommands(inputString, term);
    }
    else if ( botType == TRI_BOT )
    {
        HandleTriBotCommands(inputString, term);
    }
}

int main()
{
    // Shutdown the default simpleterm
    simpleterm_close();
    // and start up the full-duplex serial driver for the terminal
    term = fdserial_open(31, 30, 0, 9600);

    StartBotController(CONFIGURED_BOT);

    StartEyesHandler(LED_PIN);

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
                dprint(term, "%d", (int)c);
                if ((int)c == 13 || (int)c == 10)
                {
                    dprint(term, "received line:");
                    dprint(term, inputString);
                    dprint(term, "\n");
                    
                    HandleBotCommands(inputString, term);
                    HandleEyeCommands(inputString, term);
                    
                    sPos = 0;
                    inputString[0] = 0; // clear string
                }
                else if (sPos < inputStringLength - 1)
                {
                    // record next character
                    inputString[sPos] = c;
                    sPos += 1;
                    inputString[sPos] = 0; // make sure last element of string is 0
                    dprint(term, inputString);
                    dprint(term, " ok \n");
                }  
            }            
        }      
    }   
}
