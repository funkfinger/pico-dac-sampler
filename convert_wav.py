#!/usr/bin/env python3
"""
WAV to C array converter for RP2040 audio playback
Converts WAV files to C header files with embedded audio data
"""

import wave
import struct
import sys
import os

def convert_wav_to_c_array(input_file, output_file, max_duration=10.0):
    """
    Convert WAV file to C array format suitable for RP2040
    
    Args:
        input_file: Path to input WAV file
        output_file: Path to output C header file
        max_duration: Maximum duration in seconds to prevent memory issues
    """
    
    try:
        # Open the WAV file
        with wave.open(input_file, 'rb') as wav_file:
            # Get WAV file properties
            frames = wav_file.getnframes()
            sample_rate = wav_file.getframerate()
            channels = wav_file.getnchannels()
            sample_width = wav_file.getsampwidth()
            
            print(f"Original WAV properties:")
            print(f"  Sample rate: {sample_rate} Hz")
            print(f"  Channels: {channels}")
            print(f"  Sample width: {sample_width} bytes")
            print(f"  Duration: {frames / sample_rate:.2f} seconds")
            print(f"  File size: {frames * channels * sample_width} bytes")
            
            # Calculate maximum frames to fit in memory
            max_frames = int(max_duration * sample_rate)
            if frames > max_frames:
                print(f"\nWarning: File too long ({frames/sample_rate:.1f}s), truncating to {max_duration}s")
                frames = max_frames
            
            # Read audio data
            raw_audio = wav_file.readframes(frames)
            
            # Convert to 16-bit samples
            if sample_width == 1:
                # 8-bit to 16-bit
                samples = struct.unpack(f'{len(raw_audio)}B', raw_audio)
                samples = [(s - 128) * 256 for s in samples]  # Convert unsigned 8-bit to signed 16-bit
            elif sample_width == 2:
                # Already 16-bit
                samples = struct.unpack(f'{len(raw_audio)//2}h', raw_audio)
            else:
                raise ValueError(f"Unsupported sample width: {sample_width}")
            
            # Convert stereo to mono if needed (mix channels)
            if channels == 2:
                mono_samples = []
                for i in range(0, len(samples), 2):
                    # Mix left and right channels
                    mixed = (samples[i] + samples[i+1]) // 2
                    mono_samples.append(mixed)
                samples = mono_samples
                channels = 1
            
            # Downsample if needed (simple decimation)
            target_sample_rate = 16000
            if sample_rate > target_sample_rate:
                decimation_factor = sample_rate // target_sample_rate
                downsampled = samples[::decimation_factor]
                samples = downsampled
                sample_rate = target_sample_rate
                print(f"Downsampled to {target_sample_rate} Hz")
            
            print(f"\nProcessed audio:")
            print(f"  Sample rate: {sample_rate} Hz")
            print(f"  Channels: {channels}")
            print(f"  Samples: {len(samples)}")
            print(f"  Duration: {len(samples) / sample_rate:.2f} seconds")
            print(f"  Data size: {len(samples) * 2} bytes")
            
            # Create WAV header
            data_size = len(samples) * 2  # 2 bytes per 16-bit sample
            file_size = 36 + data_size
            
            wav_header = struct.pack('<4sI4s4sIHHIIHH4sI',
                b'RIFF', file_size, b'WAVE',
                b'fmt ', 16, 1, channels, sample_rate, sample_rate * channels * 2, channels * 2, 16,
                b'data', data_size
            )
            
            # Combine header and data
            wav_data = wav_header + struct.pack(f'<{len(samples)}h', *samples)
            
            # Generate C header file
            base_name = os.path.splitext(os.path.basename(input_file))[0]
            var_name = base_name.replace('-', '_').replace(' ', '_').lower()
            
            with open(output_file, 'w') as f:
                f.write(f"#ifndef {var_name.upper()}_H\n")
                f.write(f"#define {var_name.upper()}_H\n\n")
                f.write("#include <Arduino.h>\n\n")
                f.write(f"// Audio file: {os.path.basename(input_file)}\n")
                f.write(f"// Sample rate: {sample_rate} Hz, Channels: {channels}, Duration: {len(samples) / sample_rate:.2f}s\n")
                f.write(f"const uint8_t {var_name}_wav[] PROGMEM = {{\n")
                
                # Write data in rows of 12 bytes
                for i in range(0, len(wav_data), 12):
                    chunk = wav_data[i:i+12]
                    hex_values = ', '.join(f'0x{b:02X}' for b in chunk)
                    f.write(f"    {hex_values}")
                    if i + 12 < len(wav_data):
                        f.write(",")
                    f.write("\n")
                
                f.write("};\n\n")
                f.write(f"const size_t {var_name}_wav_size = sizeof({var_name}_wav);\n\n")
                f.write(f"#endif // {var_name.upper()}_H\n")
            
            print(f"\nC header file created: {output_file}")
            print(f"Variable name: {var_name}_wav")
            print(f"Size variable: {var_name}_wav_size")
            
    except Exception as e:
        print(f"Error: {e}")
        return False
    
    return True

if __name__ == "__main__":
    input_file = "source/sounds-of-mars-one-small-step-earth.wav"
    output_file = "src/mars_audio.h"
    
    if not os.path.exists(input_file):
        print(f"Error: Input file '{input_file}' not found")
        sys.exit(1)
    
    # Convert with 5 second maximum to fit in memory
    success = convert_wav_to_c_array(input_file, output_file, max_duration=5.0)
    
    if success:
        print("\nConversion completed successfully!")
        print(f"You can now include '{output_file}' in your project.")
    else:
        print("Conversion failed!")
        sys.exit(1)
