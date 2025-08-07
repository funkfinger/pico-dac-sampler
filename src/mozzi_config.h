/*
  Mozzi Configuration for RP2040 with Custom I2S Pins

  This file should be renamed to mozzi_config.h and placed in the project root
  or in the Arduino libraries folder for global Mozzi configuration.

  For PlatformIO, this can also be configured via build flags in platformio.ini
*/

#ifndef MOZZI_CONFIG_H_
#define MOZZI_CONFIG_H_

// Use I2S output mode for RP2040
#define EXTERNAL_AUDIO_OUTPUT true
#define MOZZI_AUDIO_MODE MOZZI_OUTPUT_EXTERNAL_TIMED

// Custom I2S pin definitions for RP2040
// These will be used in the main code to configure I2S manually
#define CUSTOM_I2S_BCK_PIN 26   // Bit clock pin (BCK)
#define CUSTOM_I2S_LRCK_PIN 27  // Word select pin (LCK/LRCK)
#define CUSTOM_I2S_DATA_PIN 28  // Data out pin (DIN/DOUT)

#endif  // MOZZI_CONFIG_H_
