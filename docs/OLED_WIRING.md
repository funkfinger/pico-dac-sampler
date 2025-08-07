# OLED Display Wiring Guide

## Hardware Requirements

- **0.91" SSD1306 OLED Display** (128x32 pixels, I2C interface)
- **RP2040 Pico** or compatible board
- **PCM5102A I2S DAC** (for audio output)
- **Jumper wires** for connections

## Pin Connections

### OLED Display (SSD1306)
| OLED Pin | RP2040 Pin | Function |
|----------|------------|----------|
| VCC      | 3V3        | Power (3.3V) |
| GND      | GND        | Ground |
| SDA      | GPIO4      | I2C Data |
| SCL      | GPIO5      | I2C Clock |

### I2S Audio Output (PCM5102A)
| DAC Pin  | RP2040 Pin | Function |
|----------|------------|----------|
| VCC      | 3V3        | Power (3.3V) |
| GND      | GND        | Ground |
| BCK      | GPIO26     | Bit Clock |
| LCK      | GPIO27     | Word Select (Left/Right Clock) |
| DIN      | GPIO28     | Data Input |

## I2C Configuration

- **I2C Address**: 0x3C (standard for SSD1306)
- **I2C Speed**: 400kHz (default)
- **Pull-up resistors**: Usually built-in on OLED modules

## Display Features

### Current Display Layout
```
┌─────────────────────────┐
│ Pico DAC Sampler        │
│ Mode: Sine Wave         │
│ Freq: 440 Hz            │
│                         │
└─────────────────────────┘
```

### Sample Mode Display
```
┌─────────────────────────┐
│ Pico DAC Sampler        │
│ Mode: Sample            │
│ Playing...              │
│ Progress: 45%           │
└─────────────────────────┘
```

## Display Updates

- **Mode changes**: Immediate update when switching between sine wave and sample modes
- **Frequency changes**: Real-time update when adjusting sine wave frequency
- **Sample progress**: Updates every 100ms during sample playback
- **Sample trigger**: Immediate update when spacebar is pressed

## Troubleshooting

### Display Not Working
1. **Check wiring**: Verify SDA/SCL connections to GPIO4/GPIO5
2. **Check power**: Ensure 3.3V and GND are connected
3. **Check I2C address**: Most SSD1306 displays use 0x3C
4. **Serial monitor**: Look for "OLED display initialized" message

### Display Garbled
1. **I2C interference**: Keep I2C wires short and away from I2S lines
2. **Power supply**: Ensure stable 3.3V supply
3. **Ground loops**: Use common ground for all components

### Performance Issues
1. **Update frequency**: Display updates every 100ms during sample playback
2. **Memory usage**: OLED library adds ~1KB RAM usage
3. **Processing time**: Display updates are non-blocking

## Code Integration

The OLED display is integrated into the main audio loop:

- `updateDisplay()`: Refreshes the display content
- `setup()`: Initializes I2C and OLED display
- `updateControl()`: Triggers display updates on mode/frequency changes
- `loop()`: Periodic updates for sample progress

## Future Enhancements

- **Menu system**: Navigate through different settings
- **Waveform display**: Show audio waveform in real-time
- **VU meter**: Display audio levels
- **Sample selection**: Visual sample browser
- **Parameter adjustment**: On-screen controls for speed/pitch
