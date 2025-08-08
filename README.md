# Pico DAC Sampler

A RP2040-based eurorack module for sample playback and drum machine functionality. Features multiple voice sample playback, real-time speed/pitch control, and trigger-based sequencing using the Mozzi audio synthesis library.

## 🎵 Features

### **Eurorack Module Capabilities**

- **Multi-voice drum machine** with 4 sample slots (kick, snare, hihat, tom)
- **Trigger-based sequencing** compatible with eurorack trigger signals
- **Real-time sample playback** using Mozzi audio synthesis library
- **I2S audio output** via PCM5102A DAC for high-quality audio

### **Sample Control**

- **4-button trigger system** with hardware debouncing (GPIO6-9)
- **Real-time sample triggering** for live performance
- **Individual sample assignment** per button (Kick, Snare, Hihat, Tom)
- **Eurorack-compatible** trigger inputs with protection circuitry
- **Flash memory storage** for embedded drum samples

### **Hardware Interface**

- **RP2040 microcontroller** optimized for real-time audio
- **0.91" OLED display** (128x32) for visual feedback and control
- **Eurorack-compatible** trigger inputs and CV control
- **I2S DAC output** (GPIO26=BCK, GPIO27=LCK, GPIO28=DIN)
- **I2C interface** (GPIO4=SDA, GPIO5=SCL) for OLED display
- **Serial interface** for development and configuration

## 🔧 Hardware Requirements

- **Raspberry Pi RP2040** microcontroller (Pico or compatible)
- **PCM5102A I2S DAC** for high-quality audio output
- **0.91" SSD1306 OLED Display** (128x32, I2C interface)
- **4x Momentary Push Buttons** for sample triggering
- **Protection circuitry** for eurorack trigger compatibility (optional)
- **I2S Connections**:
  - GPIO26 → BCK (Bit Clock)
  - GPIO27 → LCK (Word Select) - automatically assigned
  - GPIO28 → DIN (Data Input)
- **I2C Connections**:
  - GPIO4 → SDA (I2C Data)
  - GPIO5 → SCL (I2C Clock)
- **Button Connections**:
  - GPIO6 → Button 1 (Kick) → GND
  - GPIO7 → Button 2 (Snare) → GND
  - GPIO8 → Button 3 (Hihat) → GND
  - GPIO9 → Button 4 (Tom) → GND
- **Power**: 3.3V from eurorack power supply

## 🎚️ Controls

### **Serial Commands**

| Key     | Function                  |
| ------- | ------------------------- |
| `SPACE` | Trigger sample via serial |

### **Hardware Buttons**

| Button   | GPIO  | Function         | Sample     |
| -------- | ----- | ---------------- | ---------- |
| Button 1 | GPIO6 | Trigger Sample 1 | Kick Drum  |
| Button 2 | GPIO7 | Trigger Sample 2 | Snare Drum |
| Button 3 | GPIO8 | Trigger Sample 3 | Hi-hat     |
| Button 4 | GPIO9 | Trigger Sample 4 | Tom        |

_Hardware buttons work in both sine wave and sample modes_

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
│   ├── main.cpp              # 4-button drum machine with sample playback
│   ├── step_sample.h         # Embedded audio sample data
│   └── main_mozzi.cpp        # Backup reference file
├── source/                   # Original drum samples (to be added)
├── AI/
│   └── user_stories.md       # Project roadmap and user stories
├── convert_wav.py           # WAV to Mozzi format converter (to be updated)
└── platformio.ini           # Project configuration with Mozzi
```

## 🛠️ Development

The project uses the [Mozzi](https://github.com/sensorium/Mozzi) audio synthesis library for efficient real-time audio generation and the [arduino-pico](https://github.com/earlephilhower/arduino-pico) core for RP2040 support.

### **Current Implementation**

- `updateAudio()`: Mozzi audio generation callback (16kHz) for sample playback
- `updateControl()`: Mozzi control callback (64Hz) for button processing
- `audioOutput()`: Custom I2S output function for external DAC
- `updateButtons()`: Hardware debouncing and trigger detection
- `processButtonTriggers()`: Sample triggering logic

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
