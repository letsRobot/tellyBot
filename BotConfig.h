// BotConfig.h

#define STANDARD_BOT    0
#define TRI_BOT         1

#define NO_ACCESSORIES  0
#define PAN_TILT_ARMS   1
#define TILT_GRIPPER    2

#define EYES_WS2812     0
#define EYES_APA102     1

// Edit these lines to configure your bot
//

// uncomment ONLY one of these to select from the bots below
//#define KIT_BOT
#define DART_BOT
//#define RPE_TEST_BOT

// config for "kit"
#ifdef KIT_BOT
#define SERIAL_BITRATE          9600
#define CONFIGURED_BOT          TRI_BOT
#define CONFIGURED_ACCESSORIES  PAN_TILT_ARMS
#define CONFIGURED_EYES         EYES_WS2812
#define LED_DATA_PIN            21
#define DEFAULT_STRAIGHT_SPEED  200
#define DEFAULT_TURN_SPEED      200
#endif

#ifdef DART_BOT
#define SERIAL_BITRATE          9600
#define CONFIGURED_BOT          STANDARD_BOT
#define CONFIGURED_ACCESSORIES  TILT_GRIPPER
#define CONFIGURED_EYES         EYES_WS2812
#define LED_DATA_PIN            16
#define DEFAULT_STRAIGHT_SPEED  60
#define DEFAULT_STRAIGHT_TIMEOUT 25
#define BOOST_STRAIGHT_SPEED    200
#define DEFAULT_TURN_SPEED      30
#define DEFAULT_TURN_TIMEOUT    25
#define BOOST_TURN_SPEED        30
#define BOOST_TIME_SECONDS      30
#define USING_360_SERVOS
#endif

// bot config with APA102 (8x8 LuMini displays) eyes and faster serial bitrate
#ifdef RPE_TEST_BOT
#define SERIAL_BITRATE          115200
#define CONFIGURED_BOT          STANDARD_BOT
#define CONFIGURED_ACCESSORIES  TILT_GRIPPER
#define CONFIGURED_EYES         EYES_APA102
#define LED_DATA_PIN            8
#define LED_CLOCK_PIN           7       // the clock pin is only used if you choose EYES_APA102
#define DEFAULT_STRAIGHT_SPEED  60
#define DEFAULT_STRAIGHT_TIMEOUT 25
#define BOOST_STRAIGHT_SPEED    200
#define DEFAULT_TURN_SPEED      30
#define DEFAULT_TURN_TIMEOUT    25
#define BOOST_TURN_SPEED        30
#define BOOST_TIME_SECONDS      30
#define USING_360_SERVOS
#endif
//
// End of configuration lines
