/*
  Audio playback system for RP2040 using arduino-audio-tools
  Supports both sine wave generation and WAV file playback from flash memory

  Based on arduino-audio-tools examples
  Modified for RP2040
*/

#include <Arduino.h>
#include <math.h>

#include "AudioTools.h"
#include "mars_audio.h"

// Audio configuration
constexpr auto sample_rate = 16000;  // 16kHz sample rate
constexpr auto channels = 1;         // mono
constexpr auto bits_per_sample = 16;
constexpr auto baudrate = 115200;

// I2S pin configuration for RP2040
// BCK = GPIO26, LCK = GPIO27 (BCK+1), DIN = GPIO28
constexpr auto i2s_bck_pin = 26;
constexpr auto i2s_data_out_pin = 28;

// Audio buffer configuration
const int buffer_size = 512;  // Buffer size for audio samples

// Audio setup
AudioInfo info(sample_rate, channels, bits_per_sample);
I2SStream i2sOutput;
int16_t audio_buffer[buffer_size];

// Audio playback modes
enum AudioMode { MODE_SINE_WAVE, MODE_MARS_AUDIO };

AudioMode current_mode = MODE_SINE_WAVE;
MemoryStream mars_stream(sounds_of_mars_one_small_step_earth_wav,
                         sounds_of_mars_one_small_step_earth_wav_size);
EncodedAudioStream decoder(&i2sOutput, new WAVDecoder());
StreamCopy copier(decoder, mars_stream);

// Advanced speed/pitch control variables
float playback_speed = 1.0;  // Speed multiplier (0.1x to 4.0x)
float pitch_shift = 1.0;     // Pitch multiplier (0.5x to 2.0x)
bool independent_controls =
    false;  // false = linked speed/pitch, true = independent
uint32_t mars_sample_position = 0;
int16_t mars_audio_buffer[buffer_size];
int16_t pitch_buffer[buffer_size * 2];  // Buffer for pitch shifting

// Sine wave generator function
void generateSineWave(int16_t* buffer, int buffer_len, float frequency,
                      float amplitude, uint32_t& sample_count) {
  for (int i = 0; i < buffer_len; i++) {
    // Calculate the current phase
    float phase = 2.0 * M_PI * frequency * sample_count / sample_rate;

    // Generate sine wave sample
    float sample_float = amplitude * sin(phase);

    // Convert to 16-bit integer
    buffer[i] = (int16_t)(sample_float * 32767.0);

    sample_count++;
  }
}

// Helper function to get a single sample with bounds checking
int16_t getSampleAt(uint32_t index) {
  const uint8_t* audio_data = sounds_of_mars_one_small_step_earth_wav + 44;
  const uint32_t audio_data_size =
      sounds_of_mars_one_small_step_earth_wav_size - 44;
  const uint32_t total_samples = audio_data_size / 2;

  if (index >= total_samples) {
    index = index % total_samples;  // Wrap around
  }

  return (int16_t)(audio_data[index * 2] | (audio_data[index * 2 + 1] << 8));
}

// Linear interpolation between two samples
int16_t interpolate(int16_t sample1, int16_t sample2, float fraction) {
  return (int16_t)(sample1 + (sample2 - sample1) * fraction);
}

// Advanced function with linear interpolation for smooth speed control
void getMarsAudioSamplesInterpolated(int16_t* buffer, int buffer_len,
                                     float speed) {
  static float fractional_position = 0.0;
  const uint32_t audio_data_size =
      sounds_of_mars_one_small_step_earth_wav_size - 44;
  const uint32_t total_samples = audio_data_size / 2;

  for (int i = 0; i < buffer_len; i++) {
    // Get integer and fractional parts of position
    uint32_t sample_index = (uint32_t)fractional_position;
    float fraction = fractional_position - sample_index;

    // Handle looping
    if (sample_index >= total_samples - 1) {
      fractional_position = fractional_position - total_samples;
      sample_index = (uint32_t)fractional_position;
      fraction = fractional_position - sample_index;
    }

    // Get current and next samples
    int16_t sample1 = getSampleAt(sample_index);
    int16_t sample2 = getSampleAt(sample_index + 1);

    // Linear interpolation for smooth playback
    buffer[i] = interpolate(sample1, sample2, fraction);

    // Advance position by speed factor
    fractional_position += speed;
  }
}

// Simple pitch shifting using sample rate conversion
void pitchShiftBuffer(int16_t* input, int16_t* output, int buffer_len,
                      float pitch_ratio) {
  static float pitch_position = 0.0;

  for (int i = 0; i < buffer_len; i++) {
    // Get integer and fractional parts
    uint32_t index = (uint32_t)pitch_position;
    float fraction = pitch_position - index;

    // Handle bounds
    if (index >= buffer_len - 1) {
      pitch_position = 0.0;
      index = 0;
      fraction = 0.0;
    }

    // Interpolate between samples
    int16_t sample1 = input[index];
    int16_t sample2 = input[index + 1];
    output[i] = interpolate(sample1, sample2, fraction);

    // Advance by pitch ratio
    pitch_position += pitch_ratio;
  }
}

// Function to switch audio modes
void switchMode() {
  if (current_mode == MODE_SINE_WAVE) {
    current_mode = MODE_MARS_AUDIO;
    Serial.println("Switching to Mars audio playback...");
    mars_sample_position = 0;
  } else {
    current_mode = MODE_SINE_WAVE;
    Serial.println("Switching to sine wave generation...");
  }
}

// Function to adjust playback speed
void adjustSpeed(float delta) {
  playback_speed += delta;
  if (playback_speed < 0.1) playback_speed = 0.1;  // Minimum speed
  if (playback_speed > 4.0) playback_speed = 4.0;  // Maximum speed

  if (!independent_controls) {
    pitch_shift = playback_speed;  // Link pitch to speed
  }

  Serial.print("Playback speed: ");
  Serial.print(playback_speed, 2);
  Serial.print("x");
  if (!independent_controls) {
    Serial.println(" (pitch linked)");
  } else {
    Serial.println();
  }
}

// Function to adjust pitch independently
void adjustPitch(float delta) {
  pitch_shift += delta;
  if (pitch_shift < 0.5) pitch_shift = 0.5;  // Minimum pitch
  if (pitch_shift > 2.0) pitch_shift = 2.0;  // Maximum pitch

  Serial.print("Pitch shift: ");
  Serial.print(pitch_shift, 2);
  Serial.println("x");
}

// Function to toggle independent speed/pitch control
void toggleIndependentControls() {
  independent_controls = !independent_controls;

  if (!independent_controls) {
    pitch_shift = playback_speed;  // Sync pitch to speed
    Serial.println("Speed/Pitch LINKED - controls affect both");
  } else {
    Serial.println("Speed/Pitch INDEPENDENT - separate controls");
  }

  Serial.print("Current: Speed=");
  Serial.print(playback_speed, 2);
  Serial.print("x, Pitch=");
  Serial.print(pitch_shift, 2);
  Serial.println("x");
}

void setup() {
  // turn on LED to indicate we're running
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);

  Serial.begin(baudrate);
  delay(500);  // Give serial time to initialize

  Serial.println("Audio Player Starting...");
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // setup I2S output
  auto config = i2sOutput.defaultConfig(TX_MODE);
  config.copyFrom(info);
  config.pin_bck = i2s_bck_pin;
  config.pin_data = i2s_data_out_pin;
  // Note: pin_ws is automatically set to pin_bck + 1 on RP2040

  auto result = i2sOutput.begin(config);
  if (!result) {
    Serial.println("Failed to initialize I2S output!");
    while (1);  // do nothing
  }

  Serial.println("Advanced Audio Player Ready!");
  Serial.println("Commands:");
  Serial.println("  's' - Play sine wave (440Hz)");
  Serial.println("  'm' - Play Mars audio");
  Serial.println("  '+' - Increase speed");
  Serial.println("  '-' - Decrease speed");
  Serial.println("  'p' - Increase pitch (independent mode)");
  Serial.println("  'o' - Decrease pitch (independent mode)");
  Serial.println("  'i' - Toggle independent speed/pitch control");
  Serial.println("  '1' - Reset speed to 1.0x");
  Serial.println("  '2' - Reset pitch to 1.0x");
  Serial.println("Starting with sine wave...");
}

void loop() {
  // Check for serial commands
  if (Serial.available()) {
    char command = Serial.read();
    switch (command) {
      case 's':
      case 'S':
        if (current_mode != MODE_SINE_WAVE) {
          switchMode();
        }
        break;
      case 'm':
      case 'M':
        if (current_mode != MODE_MARS_AUDIO) {
          switchMode();
        }
        break;
      case '+':
        adjustSpeed(0.1);  // Increase speed by 0.1x
        break;
      case '-':
        adjustSpeed(-0.1);  // Decrease speed by 0.1x
        break;
      case 'p':
      case 'P':
        if (independent_controls) {
          adjustPitch(0.1);  // Increase pitch by 0.1x
        } else {
          Serial.println("Enable independent controls first (press 'i')");
        }
        break;
      case 'o':
      case 'O':
        if (independent_controls) {
          adjustPitch(-0.1);  // Decrease pitch by 0.1x
        } else {
          Serial.println("Enable independent controls first (press 'i')");
        }
        break;
      case 'i':
      case 'I':
        toggleIndependentControls();
        break;
      case '1':
        playback_speed = 1.0;
        if (!independent_controls) pitch_shift = 1.0;
        Serial.println("Speed reset to 1.0x");
        break;
      case '2':
        pitch_shift = 1.0;
        Serial.println("Pitch reset to 1.0x");
        break;
    }
  }

  // Play audio based on current mode
  if (current_mode == MODE_SINE_WAVE) {
    // Sine wave mode
    static uint32_t sample_count = 0;
    const float frequency = 440.0;  // 440Hz tone
    const float amplitude = 0.3;    // 30% amplitude to avoid clipping

    // Generate sine wave samples using our function
    generateSineWave(audio_buffer, buffer_size, frequency, amplitude,
                     sample_count);

    // Write the buffer to I2S output
    size_t bytes_written =
        i2sOutput.write((uint8_t*)audio_buffer, buffer_size * sizeof(int16_t));

    // Optional: print status occasionally
    static uint32_t last_print = 0;
    if (millis() - last_print > 5000) {  // Every 5 seconds
      Serial.print("Playing 440Hz sine wave... Samples: ");
      Serial.println(sample_count);
      last_print = millis();
    }

  } else if (current_mode == MODE_MARS_AUDIO) {
    // Mars audio mode with advanced speed/pitch control
    if (independent_controls && pitch_shift != 1.0) {
      // Independent speed and pitch control
      // First apply speed control with interpolation
      getMarsAudioSamplesInterpolated(pitch_buffer, buffer_size,
                                      playback_speed);

      // Then apply pitch shifting
      pitchShiftBuffer(pitch_buffer, mars_audio_buffer, buffer_size,
                       pitch_shift);
    } else {
      // Linked speed/pitch control (traditional)
      float combined_rate =
          independent_controls ? playback_speed : playback_speed;
      getMarsAudioSamplesInterpolated(mars_audio_buffer, buffer_size,
                                      combined_rate);
    }

    // Write the buffer to I2S output
    size_t bytes_written = i2sOutput.write((uint8_t*)mars_audio_buffer,
                                           buffer_size * sizeof(int16_t));

    // Optional: print status occasionally
    static uint32_t last_mars_print = 0;
    if (millis() - last_mars_print > 5000) {  // Every 5 seconds
      if (independent_controls) {
        Serial.print("Playing Mars audio - Speed: ");
        Serial.print(playback_speed, 2);
        Serial.print("x, Pitch: ");
        Serial.print(pitch_shift, 2);
        Serial.println("x");
      } else {
        Serial.print("Playing Mars audio at ");
        Serial.print(playback_speed, 2);
        Serial.println("x speed/pitch");
      }
      last_mars_print = millis();
    }
  }
}
