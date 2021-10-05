// a place to include the debug_print macro

#ifndef DEBUG_INCLUDED
#define DEBUG_INCLUDED

// If defined will use the serial monitor to output debug info
//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(arg)    Serial.println(arg)
#else
#define DEBUG_PRINT(arg)    // Don't do anything in release builds
#endif

#endif