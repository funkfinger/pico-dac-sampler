# RP2040 Advanced Audio Player

An advanced audio player for the Raspberry Pi RP2040 microcontroller featuring real-time speed/pitch control, WAV file playback from flash memory, and high-quality linear interpolation audio processing.

## 🎵 Features

### **Audio Playback**
- **WAV file playback** from flash memory using arduino-audio-tools
- **Real-time sine wave generation** (440Hz test tone)
- **I2S audio output** for high-quality digital audio
- **16kHz sample rate, 16-bit mono** audio processing

### **Advanced Speed/Pitch Control**
- **Real-time speed control**: 0.1x to 4.0x range (10% to 400%)
- **Independent pitch control**: 0.5x to 2.0x range (50% to 200%)
- **Linear interpolation** for smooth, high-quality audio processing
- **Dual control modes**: LINKED (traditional) and INDEPENDENT
- **Fractional sample positioning** for smooth speed changes

### **Interactive Controls**
- **Serial interface** for real-time control
- **Mode switching** between sine wave and audio playback
- **Fine control** with 0.1x increments
- **Instant parameter changes** without audio interruption

## 🔧 Hardware Requirements

- **Raspberry Pi RP2040** microcontroller
- **I2S DAC/Amplifier** (e.g., MAX98357A)
- **Connections**:
  - GPIO26 → BCK (Bit Clock)
  - GPIO27 → LCK (Word Select) - automatically assigned
  - GPIO28 → DIN (Data Input)

## 🎚️ Controls

| Key | Function |
|-----|----------|
| `s` | Switch to sine wave mode |
| `m` | Switch to Mars audio mode |
| `+` | Increase speed by 0.1x |
| `-` | Decrease speed by 0.1x |
| `p` | Increase pitch by 0.1x (independent mode) |
| `o` | Decrease pitch by 0.1x (independent mode) |
| `i` | Toggle LINKED ↔ INDEPENDENT control modes |
| `1` | Reset speed to 1.0x |
| `2` | Reset pitch to 1.0x |

## 🚀 Getting Started

### **Prerequisites**
- [PlatformIO](https://platformio.org/) installed
- RP2040 development board
- I2S DAC/amplifier module

### **Installation**
1. Clone this repository:
   ```bash
   git clone https://github.com/funkfinger/rp2040-advanced-audio-player.git
   cd rp2040-advanced-audio-player
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
│   ├── main.cpp              # Main audio player code
│   ├── mars_audio.h          # Embedded Mars audio data
│   └── audio_data.h          # Test audio data
├── source/                   # Original audio files
├── convert_wav.py           # WAV to C array converter
├── advanced_speed_control.md # Detailed documentation
└── platformio.ini           # Project configuration
```

## 🛠️ Development

The project uses the [arduino-audio-tools](https://github.com/pschatzmann/arduino-audio-tools) library for robust audio processing and the [arduino-pico](https://github.com/earlephilhower/arduino-pico) core for RP2040 support.

### **Key Functions**
- `generateSineWave()`: Reusable sine wave generator
- `getMarsAudioSamplesInterpolated()`: High-quality speed control with interpolation
- `pitchShiftBuffer()`: Independent pitch shifting
- `adjustSpeed()` / `adjustPitch()`: Real-time parameter control

## 📄 License

This project is open source. Feel free to use, modify, and distribute.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

---

**Built with ❤️ for the RP2040 community**
