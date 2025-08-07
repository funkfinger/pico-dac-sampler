# Pico DAC Sampler

A RP2040-based eurorack module for sample playback and drum machine functionality. Features multiple voice sample playback, real-time speed/pitch control, and trigger-based sequencing using the Mozzi audio synthesis library.

## 🎵 Features

### **Eurorack Module Capabilities**

- **Multi-voice drum machine** with 4 sample slots (kick, snare, hihat, tom)
- **Trigger-based sequencing** compatible with eurorack trigger signals
- **Real-time sample playback** using Mozzi audio synthesis library
- **I2S audio output** via PCM5102A DAC for high-quality audio

### **Sample Control**

- **Real-time speed control** for pitch shifting and time stretching
- **Individual sample tuning** for each voice
- **Simultaneous playback** of multiple samples (polyphonic)
- **Flash memory storage** for embedded drum samples

### **Hardware Interface**

- **RP2040 microcontroller** optimized for real-time audio
- **Eurorack-compatible** trigger inputs and CV control
- **I2S DAC output** (GPIO26=BCK, GPIO27=LCK, GPIO28=DIN)
- **Serial interface** for development and configuration

## 🔧 Hardware Requirements

- **Raspberry Pi RP2040** microcontroller (Pico or compatible)
- **PCM5102A I2S DAC** for high-quality audio output
- **Eurorack-compatible** trigger inputs (optional)
- **I2S Connections**:
  - GPIO26 → BCK (Bit Clock)
  - GPIO27 → LCK (Word Select) - automatically assigned
  - GPIO28 → DIN (Data Input)
- **Power**: 3.3V from eurorack power supply

## 🎚️ Current Controls (Development Mode)

| Key   | Function                                    |
| ----- | ------------------------------------------- |
| `1-9` | Change sine wave frequency (100Hz to 900Hz) |
| `0`   | Reset to 440Hz (A4 note)                    |

_Note: Full drum machine controls will be added as development progresses_

## 🚀 Getting Started

### **Prerequisites**

- [PlatformIO](https://platformio.org/) installed
- RP2040 development board
- I2S DAC/amplifier module

### **Installation**

1. Clone this repository:

   ```bash
   git clone https://github.com/funkfinger/pico-dac-sampler.git
   cd pico-dac-sampler
   ```

2. Build and upload:

   ```bash
   pio run --target upload
   ```

3. Open serial monitor:
   ```bash
   pio device monitor --baud 115200
   ```

### **Adding Your Own Audio**

1. Place your WAV file in the `source/` directory
2. Run the conversion script:
   ```bash
   python3 convert_wav.py
   ```
3. Include the generated header file in `main.cpp`

## 📊 Technical Specifications

- **Memory Usage**: 9.2% RAM, 13.7% Flash
- **Audio Quality**: Linear interpolation with fractional positioning
- **Buffer Size**: 512 samples for smooth playback
- **Sample Rate**: 16kHz (automatically converted from source)
- **Bit Depth**: 16-bit signed integers
- **Channels**: Mono (stereo automatically mixed to mono)

## 🎯 Creative Applications

### **LINKED Mode** (Speed + Pitch together)

- **0.5x**: Classic slow-motion with lower pitch
- **2.0x**: Fast playback with higher pitch (chipmunk effect)

### **INDEPENDENT Mode** (Separate controls)

- **Speed 0.5x, Pitch 1.0x**: Slow motion with normal pitch
- **Speed 1.0x, Pitch 1.5x**: Normal speed with higher pitch
- **Speed 2.0x, Pitch 0.8x**: Fast playback with lower pitch

## 📁 Project Structure

```
├── src/
│   ├── main.cpp              # Mozzi-based sine wave generator
│   ├── mozzi_config.h        # Mozzi configuration for I2S
│   ├── mars_audio.h          # Legacy audio data (to be converted)
│   └── audio_data.h          # Legacy test data
├── source/                   # Original drum samples (to be added)
├── AI/
│   └── user_stories.md       # Project roadmap and user stories
├── convert_wav.py           # WAV to Mozzi format converter (to be updated)
└── platformio.ini           # Project configuration with Mozzi
```

## 🛠️ Development

The project uses the [Mozzi](https://github.com/sensorium/Mozzi) audio synthesis library for efficient real-time audio generation and the [arduino-pico](https://github.com/earlephilhower/arduino-pico) core for RP2040 support.

### **Current Implementation**

- `updateAudio()`: Mozzi audio generation callback (16kHz)
- `updateControl()`: Mozzi control callback (64Hz) for parameter changes
- `audioOutput()`: Custom I2S output function for external DAC
- Sine wave oscillator using Mozzi's optimized wavetables

### **Planned Features**

- Multi-voice sample playback engine
- Trigger input handling for eurorack compatibility
- Real-time speed/pitch control per voice
- CV input processing for parameter modulation

## 📄 License

This project is open source. Feel free to use, modify, and distribute.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

---

**Built with ❤️ for the eurorack and RP2040 communities**
