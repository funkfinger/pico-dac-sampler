/*
  Pico DAC Sampler - RP2040 Eurorack Drum Machine Module

  Development Phase 1: Mozzi Sine Wave Generator
  Generates a 440Hz sine wave using Mozzi audio library with I2S output

  Hardware: RP2040 with PCM5102A I2S DAC
  I2S Pins: BCK=GPIO26, LCK=GPIO27, DIN=GPIO28

  Next: Multi-voice sample playback for drum machine functionality
*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <I2S.h>    // For I2S output on RP2040
#include <Mozzi.h>  // Use Mozzi.h instead of MozziGuts.h for Mozzi 2.0
#include <Wire.h>

#include "hihat_sample.h"  // Hi-hat sample
#include "kick_sample.h"   // Kick drum sample
#include "snare_sample.h"  // Snare drum sample
#include "tom_sample.h"    // Tom sample

// I2S configuration for custom pins
#define I2S_BCK_PIN 26   // Bit clock
#define I2S_LCK_PIN 27   // Word select (automatically BCK+1 on RP2040)
#define I2S_DATA_PIN 28  // Data output

// OLED configuration
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT \
  32                   // OLED display height, in pixels (0.91" is usually 32)
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // I2C address for SSD1306 (usually 0x3C)

// I2C pins for OLED (using default Wire pins)
#define SDA_PIN 4  // GPIO4 for I2C SDA
#define SCL_PIN 5  // GPIO5 for I2C SCL

// Button/Trigger input pins (with future eurorack compatibility)
#define BUTTON_1_PIN 6  // GPIO6 - Sample 1 (Kick)
#define BUTTON_2_PIN 7  // GPIO7 - Sample 2 (Snare)
#define BUTTON_3_PIN 8  // GPIO8 - Sample 3 (Hihat)
#define BUTTON_4_PIN 9  // GPIO9 - Sample 4 (Tom)

// Debounce settings
#define DEBOUNCE_DELAY 20    // 20ms debounce delay
#define TRIGGER_MIN_PULSE 5  // Minimum 5ms pulse for eurorack triggers

// Multi-voice sample player structure
struct SamplePlayer {
  const int8_t* data;
  uint32_t length;
  uint32_t position;
  bool playing;
  const char* name;
};

// Initialize sample players for each drum
SamplePlayer samplePlayers[4] = {
    {kick_sample_data, kick_sample_length, 0, false, "Kick"},
    {snare_sample_data, snare_sample_length, 0, false, "Snare"},
    {hihat_sample_data, hihat_sample_length, 0, false, "Hihat"},
    {tom_sample_data, tom_sample_length, 0, false, "Tom"}};

// Track last triggered sample for display
int lastTriggeredSample = 0;

// Create OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// I2S output object
I2S i2s(OUTPUT, I2S_BCK_PIN, I2S_DATA_PIN);

// Control variables
bool oledWorking = false;  // Track if OLED is functional

// Button/Trigger state tracking
struct ButtonState {
  int pin;
  bool lastState;
  bool currentState;
  unsigned long lastDebounceTime;
  bool triggered;
  const char* name;
};

// Initialize button states for 4 sample triggers
ButtonState buttons[4] = {{BUTTON_1_PIN, HIGH, HIGH, 0, false, "Kick"},
                          {BUTTON_2_PIN, HIGH, HIGH, 0, false, "Snare"},
                          {BUTTON_3_PIN, HIGH, HIGH, 0, false, "Hihat"},
                          {BUTTON_4_PIN, HIGH, HIGH, 0, false, "Tom"}};

// Sample playback state (will expand for multi-voice later)
int currentSampleIndex = 0;  // Which sample to play (0-3)

// Forward declarations
void updateDisplay();

// Required audioOutput function for Mozzi 2.0 external audio mode
void audioOutput(const AudioOutput f) {
  // Convert Mozzi's mono output to stereo for I2S
  int16_t sample =
      f.l();  // Get the left channel sample from Mozzi (note the parentheses)

  // Write stereo samples (same sample for both channels)
  i2s.write16(sample, sample);
}

// Button debouncing and trigger detection
void updateButtons() {
  for (int i = 0; i < 4; i++) {
    int reading = digitalRead(buttons[i].pin);

    // Check if button state changed (for debouncing)
    if (reading != buttons[i].lastState) {
      buttons[i].lastDebounceTime = millis();
    }

    // If enough time has passed since last state change
    if ((millis() - buttons[i].lastDebounceTime) > DEBOUNCE_DELAY) {
      // If the button state has actually changed
      if (reading != buttons[i].currentState) {
        buttons[i].currentState = reading;

        // Trigger on falling edge (button press) - active LOW with pullup
        if (buttons[i].currentState == LOW) {
          buttons[i].triggered = true;
          Serial.print("Button ");
          Serial.print(i + 1);
          Serial.print(" (");
          Serial.print(buttons[i].name);
          Serial.println(") triggered!");
        }
      }
    }

    buttons[i].lastState = reading;
  }
}

// Process button triggers and start sample playback
void processButtonTriggers() {
  for (int i = 0; i < 4; i++) {
    if (buttons[i].triggered) {
      buttons[i].triggered = false;  // Clear trigger flag

      // Trigger the corresponding sample
      samplePlayers[i].position = 0;
      samplePlayers[i].playing = true;
      lastTriggeredSample = i;

      Serial.print("Playing ");
      Serial.print(samplePlayers[i].name);
      Serial.print(" (Button ");
      Serial.print(i + 1);
      Serial.println(")");

      // Update display if OLED is working
      if (oledWorking) {
        updateDisplay();
      }
    }
  }
}

// Display functions
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // Title
  display.println("Pico DAC Sampler");

  // Show playing samples
  bool anyPlaying = false;
  for (int i = 0; i < 4; i++) {
    if (samplePlayers[i].playing) {
      anyPlaying = true;
      break;
    }
  }

  if (anyPlaying) {
    display.println("Playing:");
    for (int i = 0; i < 4; i++) {
      if (samplePlayers[i].playing) {
        display.print(samplePlayers[i].name);
        display.print(" ");
      }
    }
    display.println();
  } else {
    display.println("Ready");
    display.print("Last: ");
    display.println(samplePlayers[lastTriggeredSample].name);
  }

  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Pico DAC Sampler Starting...");

  // Initialize button pins with internal pull-up resistors
  for (int i = 0; i < 4; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    Serial.print("Initialized button ");
    Serial.print(i + 1);
    Serial.print(" (");
    Serial.print(buttons[i].name);
    Serial.print(") on GPIO");
    Serial.println(buttons[i].pin);
  }

  // Initialize I2C for OLED
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    // Continue without display rather than stopping
  } else {
    Serial.println("OLED display initialized");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Pico DAC Sampler");
    display.println("Initializing...");
    display.display();
    delay(1000);
  }

  // Initialize I2S with 16-bit samples
  i2s.setBitsPerSample(16);

  // Start I2S at the configured sample rate
  if (!i2s.begin(MOZZI_AUDIO_RATE)) {
    Serial.println("Failed to initialize I2S!");
    while (1);  // Stop execution
  }

  // Initialize Mozzi (will use external audio output via audioOutput function)
  startMozzi();

  Serial.println("Pico DAC Sampler initialized - 4-button drum machine!");
  Serial.println("Commands:");
  Serial.println("  SPACE: Trigger sample via serial");
  Serial.println("Hardware Buttons:");
  Serial.println("  Button 1 (GPIO6): Kick sample");
  Serial.println("  Button 2 (GPIO7): Snare sample");
  Serial.println("  Button 3 (GPIO8): Hihat sample");
  Serial.println("  Button 4 (GPIO9): Tom sample");
  Serial.println("Ready for button triggers...");

  // Update display with initial state
  updateDisplay();
}

void updateControl() {
  // This function is called at CONTROL_RATE (64Hz)
  // Handle any control changes here

  // Process button inputs with debouncing
  updateButtons();
  processButtonTriggers();

  // Check for serial input
  if (Serial.available()) {
    char input = Serial.read();

    switch (input) {
      case ' ':  // Spacebar to trigger last sample
        samplePlayers[lastTriggeredSample].position = 0;
        samplePlayers[lastTriggeredSample].playing = true;
        Serial.print("Sample triggered via spacebar: ");
        Serial.println(samplePlayers[lastTriggeredSample].name);
        updateDisplay();
        break;
      default:
        // Ignore other input
        break;
    }
  }
}

AudioOutput updateAudio() {
  // This function is called at AUDIO_RATE (16384Hz)
  // Return the next audio sample as AudioOutput

  int16_t mixedSample = 0;  // Use 16-bit for mixing to prevent overflow

  // Mix all playing samples
  for (int i = 0; i < 4; i++) {
    if (samplePlayers[i].playing &&
        samplePlayers[i].position < samplePlayers[i].length) {
      // Read sample from PROGMEM and add to mix
      int8_t sample =
          pgm_read_byte(&samplePlayers[i].data[samplePlayers[i].position]);
      mixedSample += sample;
      samplePlayers[i].position++;
    } else if (samplePlayers[i].playing) {
      // Sample finished playing
      samplePlayers[i].playing = false;
    }
  }

  // Clamp mixed sample to 8-bit range and convert
  mixedSample = max(-128, min(127, mixedSample));

  return MonoOutput::from8Bit((int8_t)mixedSample);
}

void loop() {
  // audioHook() must be called regularly for Mozzi to work
  audioHook();

  // Update display periodically to show sample progress
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 100) {  // Every 100ms
    // Check if any samples are playing
    bool anyPlaying = false;
    for (int i = 0; i < 4; i++) {
      if (samplePlayers[i].playing) {
        anyPlaying = true;
        break;
      }
    }
    if (anyPlaying) {
      updateDisplay();  // Update display when samples are playing
    }
    lastDisplayUpdate = millis();
  }

  // Optional: Print status occasionally
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 5000) {  // Every 5 seconds
    Serial.println("Pico DAC Sampler - Ready for button triggers");
    lastPrint = millis();
  }
}
