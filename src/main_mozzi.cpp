/*
  Minimal Mozzi Sine Wave Example for RP2040
  Generates a 440Hz sine wave using Mozzi audio library with I2S output

  Hardware: RP2040 with I2S DAC/Amplifier
  I2S Pins: BCK=GPIO26, LCK=GPIO27, DIN=GPIO28
*/

#include <Arduino.h>
#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>

// Audio configuration
#define CONTROL_RATE \
  64  // Control rate in Hz (how often updateControl() is called)
#define AUDIO_RATE 16384  // Audio sample rate (16kHz)

// Create a sine wave oscillator using Mozzi's built-in sine table
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> sineWave(SIN2048_DATA);

// Frequency control variable
float frequency = 440.0;  // 440Hz (A4 note)

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Mozzi Sine Wave Generator Starting...");

  // Initialize Mozzi with I2S output
  // Note: Mozzi will use default I2S pins, we may need to configure for
  // GPIO26/27/28
  startMozzi(CONTROL_RATE);

  // Set the frequency of the sine wave
  sineWave.setFreq(frequency);

  Serial.println("Mozzi initialized - playing 440Hz sine wave");
  Serial.println("Commands:");
  Serial.println("  1-9: Change frequency (100Hz to 900Hz)");
  Serial.println("  0: Reset to 440Hz");
}

void updateControl() {
  // This function is called at CONTROL_RATE (64Hz)
  // Handle any control changes here

  // Check for serial input to change frequency
  if (Serial.available()) {
    char input = Serial.read();

    switch (input) {
      case '1':
        frequency = 100;
        break;
      case '2':
        frequency = 200;
        break;
      case '3':
        frequency = 300;
        break;
      case '4':
        frequency = 400;
        break;
      case '5':
        frequency = 500;
        break;
      case '6':
        frequency = 600;
        break;
      case '7':
        frequency = 700;
        break;
      case '8':
        frequency = 800;
        break;
      case '9':
        frequency = 900;
        break;
      case '0':
        frequency = 440;
        break;  // Reset to A4
      default:
        return;  // Invalid input, don't change frequency
    }

    // Update the oscillator frequency
    sineWave.setFreq(frequency);

    Serial.print("Frequency changed to: ");
    Serial.print(frequency);
    Serial.println(" Hz");
  }
}

int updateAudio() {
  // This function is called at AUDIO_RATE (16384Hz)
  // Return the next audio sample
  return sineWave.next();
}

void loop() {
  // audioHook() must be called regularly for Mozzi to work
  audioHook();

  // Optional: Print status occasionally
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 5000) {  // Every 5 seconds
    Serial.print("Playing sine wave at ");
    Serial.print(frequency);
    Serial.println(" Hz");
    lastPrint = millis();
  }
}
