#!/usr/bin/env python3
"""
WAV to Mozzi AudioSample converter for Pico DAC Sampler
Converts WAV files to Mozzi-compatible C header files for sample playback
Supports both raw and Huffman-encoded formats
"""

import wave
import struct
import sys
import os
import numpy as np

def convert_wav_to_mozzi_sample(input_file, output_file, max_duration=5.0, sample_name=None):
    """
    Convert WAV file to Mozzi AudioSample format

    Args:
        input_file: Path to input WAV file
        output_file: Path to output C header file
        max_duration: Maximum duration in seconds to prevent memory issues
        sample_name: Name for the sample variable (auto-generated if None)
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
            
            # Downsample to Mozzi's preferred rate (16384 Hz)
            target_sample_rate = 16384  # Mozzi's AUDIO_RATE
            if sample_rate != target_sample_rate:
                # Use numpy for better resampling
                samples_array = np.array(samples, dtype=np.float32)
                resample_ratio = target_sample_rate / sample_rate
                new_length = int(len(samples_array) * resample_ratio)

                # Simple linear interpolation resampling
                old_indices = np.linspace(0, len(samples_array) - 1, new_length)
                samples = np.interp(old_indices, np.arange(len(samples_array)), samples_array)
                samples = samples.astype(np.int16)
                sample_rate = target_sample_rate
                print(f"Resampled to {target_sample_rate} Hz")

            # Convert to 8-bit signed for Mozzi (more memory efficient)
            samples_8bit = []
            for sample in samples:
                # Convert 16-bit to 8-bit signed (-128 to 127)
                sample_8bit = int(sample / 256)  # Divide by 256 to go from 16-bit to 8-bit
                sample_8bit = max(-128, min(127, sample_8bit))  # Clamp to 8-bit range
                samples_8bit.append(sample_8bit)

            samples = samples_8bit

            print(f"\nProcessed audio:")
            print(f"  Sample rate: {sample_rate} Hz")
            print(f"  Channels: {channels}")
            print(f"  Samples: {len(samples)}")
            print(f"  Duration: {len(samples) / sample_rate:.2f} seconds")
            print(f"  Data size: {len(samples)} bytes (8-bit)")
            print(f"  Bit depth: 8-bit signed (-128 to 127)")
            
            # Generate Mozzi-compatible C header file
            if sample_name is None:
                base_name = os.path.splitext(os.path.basename(input_file))[0]
                sample_name = base_name.replace('-', '_').replace(' ', '_').replace('.', '_').upper()

            var_name = sample_name.lower()

            with open(output_file, 'w') as f:
                f.write(f"#ifndef {sample_name}_H\n")
                f.write(f"#define {sample_name}_H\n\n")
                f.write("#include <Arduino.h>\n\n")
                f.write(f"// Mozzi AudioSample data for: {os.path.basename(input_file)}\n")
                f.write(f"// Sample rate: {sample_rate} Hz, Duration: {len(samples) / sample_rate:.2f}s\n")
                f.write(f"// Format: 8-bit signed PCM (-128 to 127)\n")
                f.write(f"// Generated by Pico DAC Sampler WAV converter\n\n")

                # Write the sample data array
                f.write(f"const int8_t {var_name}_data[] PROGMEM = {{\n")

                # Write data in rows of 16 bytes for readability
                for i in range(0, len(samples), 16):
                    chunk = samples[i:i+16]
                    # Format as signed integers
                    hex_values = ', '.join(f'{s:4d}' for s in chunk)
                    f.write(f"    {hex_values}")
                    if i + 16 < len(samples):
                        f.write(",")
                    f.write("\n")

                f.write("};\n\n")

                # Write the sample length
                f.write(f"const unsigned int {var_name}_length = {len(samples)};\n")
                f.write(f"const unsigned int {var_name}_samplerate = {sample_rate};\n\n")

                f.write(f"#endif // {sample_name}_H\n")

            print(f"\nMozzi sample header created: {output_file}")
            print(f"Sample data: {var_name}_data[]")
            print(f"Sample length: {var_name}_length")
            print(f"Sample rate: {var_name}_samplerate")
            
    except Exception as e:
        print(f"Error: {e}")
        return False
    
    return True

if __name__ == "__main__":
    # Convert Step audio to Mozzi format
    input_file = "source/one-small-step.wav"
    output_file = "src/step_sample.h"

    if not os.path.exists(input_file):
        print(f"Error: Input file '{input_file}' not found")
        sys.exit(1)

    # Convert with 3 second maximum to fit in memory (shorter for drum samples)
    success = convert_wav_to_mozzi_sample(input_file, output_file, max_duration=3.0, sample_name="STEP_SAMPLE")

    if success:
        print("\nMozzi sample conversion completed successfully!")
        print(f"Include '{output_file}' in your Mozzi project.")
        print("Use AudioSample<step_sample_length> step_sample(step_sample_data);")
    else:
        print("Conversion failed!")
        sys.exit(1)
