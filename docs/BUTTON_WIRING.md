# 4-Button Sample Trigger Wiring Guide

## Hardware Requirements

- **4x Momentary Push Buttons** (normally open, SPST)
- **RP2040 Pico** or compatible board
- **Jumper wires** for connections
- **Optional**: Pull-up resistors (10kΩ) if not using internal pull-ups

## Pin Connections

### Button Connections (Active LOW with Internal Pull-ups)
| Button | RP2040 Pin | Function | Sample Type |
|--------|------------|----------|-------------|
| Button 1 | GPIO6 | Sample Trigger 1 | Kick Drum |
| Button 2 | GPIO7 | Sample Trigger 2 | Snare Drum |
| Button 3 | GPIO8 | Sample Trigger 3 | Hi-hat |
| Button 4 | GPIO9 | Sample Trigger 4 | Tom |

### Basic Button Wiring (Recommended)
```
Button 1: GPIO6 ──┤ ├── GND
Button 2: GPIO7 ──┤ ├── GND  
Button 3: GPIO8 ──┤ ├── GND
Button 4: GPIO9 ──┤ ├── GND
```

**Note**: Internal pull-up resistors are enabled in software, so no external pull-ups needed.

## Eurorack Trigger Input Protection (Future Expansion)

For eurorack compatibility, add protection circuitry:

### Protection Circuit per Input
```
Eurorack Trigger Input (+5V to +12V)
    │
    ├── 10kΩ Resistor ──┬── RP2040 GPIO Pin
    │                   │
    └── Schottky Diode ─┴── GND (BAT54 or similar)
```

### Component Specifications
- **Series Resistor**: 10kΩ (current limiting)
- **Protection Diode**: Schottky diode (BAT54, 1N5817, etc.)
- **Voltage Rating**: Handle +12V eurorack signals
- **Current Limiting**: Protects RP2040 GPIO (max 3.3V input)

## Software Features

### Debouncing
- **Hardware-independent**: Software debouncing with 20ms delay
- **Reliable triggering**: Eliminates mechanical switch bounce
- **Eurorack compatible**: Works with both buttons and trigger pulses

### Button State Tracking
- **Individual state machines** for each button
- **Edge detection**: Triggers on falling edge (button press)
- **Non-blocking**: Processed at 64Hz control rate
- **Serial feedback**: Debug output for each trigger

### Display Integration
- **Real-time feedback**: Shows which sample is playing
- **Progress indication**: Visual progress bar during playback
- **Sample identification**: Displays sample name (Kick, Snare, etc.)

## Pin Usage Optimization

Current pin allocation is designed for future expansion:

### Used Pins
- **GPIO4, GPIO5**: I2C (OLED display)
- **GPIO6-9**: Button inputs (4 pins)
- **GPIO26-28**: I2S audio output (3 pins)
- **Total used**: 9 pins out of 26 available

### Available for Future Features
- **GPIO0-3**: Available for additional controls
- **GPIO10-25**: Available for CV inputs, additional triggers
- **ADC pins (26-29)**: Available for analog CV processing

## Troubleshooting

### Buttons Not Responding
1. **Check wiring**: Verify button connects GPIO to GND
2. **Serial monitor**: Look for "Button X triggered!" messages
3. **Pull-up resistors**: Internal pull-ups are enabled in software
4. **Debounce timing**: 20ms debounce should handle most switches

### False Triggers
1. **Increase debounce delay**: Modify `DEBOUNCE_DELAY` in code
2. **Check connections**: Loose wires can cause false triggers
3. **EMI interference**: Keep button wires away from I2S lines

### Eurorack Integration Issues
1. **Voltage protection**: Ensure protection diodes are installed
2. **Signal levels**: Eurorack triggers are +5V to +12V
3. **Pulse width**: Minimum 5ms pulse width for reliable detection
4. **Ground loops**: Use common ground for all eurorack connections

## Code Configuration

### Debounce Settings
```cpp
#define DEBOUNCE_DELAY 20    // 20ms debounce delay
#define TRIGGER_MIN_PULSE 5  // Minimum 5ms pulse for eurorack
```

### Button Pin Definitions
```cpp
#define BUTTON_1_PIN 6   // GPIO6 - Sample 1 (Kick)
#define BUTTON_2_PIN 7   // GPIO7 - Sample 2 (Snare)  
#define BUTTON_3_PIN 8   // GPIO8 - Sample 3 (Hihat)
#define BUTTON_4_PIN 9   // GPIO9 - Sample 4 (Tom)
```

## Future Enhancements

### Hardware Expansion
- **CV inputs**: Add analog control for sample parameters
- **Gate outputs**: Trigger other eurorack modules
- **LED indicators**: Visual feedback for each trigger
- **Encoder**: Parameter adjustment without serial interface

### Software Features
- **Velocity sensitivity**: Variable trigger levels
- **Sample selection**: Choose different samples per button
- **Pattern sequencing**: Built-in drum patterns
- **MIDI integration**: MIDI trigger input/output

## Performance

### Memory Usage
- **Button state tracking**: ~100 bytes RAM
- **Debounce processing**: Minimal CPU overhead
- **Real-time performance**: 64Hz control rate processing
- **No audio interruption**: Button processing doesn't affect audio

### Timing Specifications
- **Debounce delay**: 20ms (configurable)
- **Control rate**: 64Hz (15.6ms intervals)
- **Response time**: <35ms from button press to sample trigger
- **Eurorack pulse**: 5ms minimum pulse width detection
