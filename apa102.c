// apa102.c
// Driver for APA102 RGB LEDs (e.g. sparkfun LuMi & LuMini RGB LED boards)
// by Roy Eltham

#include <propeller.h>        // Propeller-specific functions

uint32_t _apa102ClkPinMask;
uint32_t _apa102DataPinMask;

void InitApa102(int clkPin, int dataPin)
{
    _apa102ClkPinMask = 1 << clkPin;
    _apa102DataPinMask = 1 << dataPin;

    // set data and clk as outputs with the clk in a low state
    DIRA |= _apa102DataPinMask;
    OUTA &= (~_apa102ClkPinMask);
    DIRA |= _apa102ClkPinMask;
}
    
__attribute__((fcache))                    // allows function to run directly from cog ram, 10x+ speed increase
void _apa102WriteLeds(const uint32_t* leds, int numLeds)
{
    OUTA &= (~_apa102ClkPinMask);
    for (int j = 0; j < numLeds; j++)
    {
        for (int i = 31; i >= 0 ; i--)
        {
            if ((leds[j] >> i) & 1)
            {
                OUTA |= _apa102DataPinMask;
            }                
            else
            {
                OUTA &= (~_apa102DataPinMask);
            }                
            OUTA ^= _apa102ClkPinMask;
            OUTA ^= _apa102ClkPinMask;
        }
    }    
}

// write out numBits zeros
__attribute__((fcache))                    // allows function to run directly from cog ram, 10x+ speed increase
void _apa102StartEndFrame(int numBits)
{
    OUTA &= (~_apa102ClkPinMask);
    OUTA &= (~_apa102DataPinMask);
    for (int i = 0; i < numBits ; i++)
    {
        OUTA ^= _apa102ClkPinMask;
        OUTA ^= _apa102ClkPinMask;
    }
}

void SendLeds(const uint32_t* leds, int numLeds)
{
    _apa102StartEndFrame(32);
    _apa102WriteLeds( leds, numLeds );
    _apa102StartEndFrame(64);
}    

