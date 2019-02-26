// BotConfig.h

#define STANDARD_BOT    0
#define TRI_BOT         1

#define EYES_WS2812     0
#define EYES_APA102     1

// Edit these lines to configure your bot
//
// config for "kit"
#define SERIAL_BITRATE          9600
#define CONFIGURED_BOT          TRI_BOT
#define CONFIGURED_EYES         EYES_WS2812
#define LED_DATA_PIN            21
#define DEFAULT_STRAIGHT_SPEED  200
#define DEFAULT_TURN_SPEED      200

//
// End of configuration lines

// example standard bot config with APA102 (8x8 LuMini displays) eyes and faster serial bitrate
/*
#define SERIAL_BITRATE          115200
#define CONFIGURED_BOT          STANDARD_BOT
#define CONFIGURED_EYES         EYES_APA102
#define LED_DATA_PIN            8
#define LED_CLOCK_PIN           7       // the clock pin is only used if you choose EYES_APA102
#define DEFAULT_STRAIGHT_SPEED  60
#define DEFAULT_TURN_SPEED      20
*/
