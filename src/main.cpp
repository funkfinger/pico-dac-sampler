/*
  Pico DAC Sampler - RP2040 Eurorack Drum Machine Module

  Development Phase 1: Mozzi Sine Wave Generator
  Generates a 440Hz sine wave using Mozzi audio library with I2S output

  Hardware: RP2040 with PCM5102A I2S DAC
  I2S Pins: BCK=GPIO26, LCK=GPIO27, DIN=GPIO28

  Next: Multi-voice sample playback for drum machine functionality
*/

#include <Arduino.h>
#include <I2S.h>    // For I2S output on RP2040
#include <Mozzi.h>  // Use Mozzi.h instead of MozziGuts.h for Mozzi 2.0
#include <Oscil.h>
#include <tables/sin2048_int8.h>

// I2S configuration for custom pins
#define I2S_BCK_PIN 26   // Bit clock
#define I2S_LCK_PIN 27   // Word select (automatically BCK+1 on RP2040)
#define I2S_DATA_PIN 28  // Data output

// Create a sine wave oscillator using Mozzi's built-in sine table
Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> sineWave(SIN2048_DATA);

// I2S output object
I2S i2s(OUTPUT, I2S_BCK_PIN, I2S_DATA_PIN);

// Frequency control variable
float frequency = 440.0;  // 440Hz (A4 note)

// Required audioOutput function for Mozzi 2.0 external audio mode
void audioOutput(const AudioOutput f) {
  // Convert Mozzi's mono output to stereo for I2S
  int16_t sample =
      f.l();  // Get the left channel sample from Mozzi (note the parentheses)

  // Write stereo samples (same sample for both channels)
  i2s.write16(sample, sample);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Mozzi Sine Wave Generator Starting...");

  // Initialize I2S with 16-bit samples
  i2s.setBitsPerSample(16);

  // Start I2S at the configured sample rate
  if (!i2s.begin(MOZZI_AUDIO_RATE)) {
    Serial.println("Failed to initialize I2S!");
    while (1);  // Stop execution
  }

  // Initialize Mozzi (will use external audio output via audioOutput function)
  startMozzi();

  // Set the frequency of the sine wave
  sineWave.setFreq(frequency);

  Serial.println("Mozzi initialized - playing 440Hz sine wave via I2S");
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

AudioOutput updateAudio() {
  // This function is called at AUDIO_RATE (16384Hz)
  // Return the next audio sample as AudioOutput
  return MonoOutput::from8Bit(sineWave.next());
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
