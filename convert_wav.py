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
            elif sample_width == 3:
                # 24-bit to 16-bit conversion
                samples = []
                for i in range(0, len(raw_audio), 3):
                    if i + 2 < len(raw_audio):
                        # Read 3 bytes (little-endian) and convert to 24-bit signed
                        byte1, byte2, byte3 = raw_audio[i], raw_audio[i+1], raw_audio[i+2]
                        sample_24bit = byte1 | (byte2 << 8) | (byte3 << 16)
                        # Convert to signed if MSB is set
                        if sample_24bit >= 0x800000:
                            sample_24bit -= 0x1000000
                        # Convert 24-bit to 16-bit by shifting right 8 bits
                        sample_16bit = sample_24bit >> 8
                        # Clamp to 16-bit range
                        sample_16bit = max(-32768, min(32767, sample_16bit))
                        samples.append(sample_16bit)
            elif sample_width == 4:
                # 32-bit to 16-bit conversion
                samples_32bit = struct.unpack(f'{len(raw_audio)//4}i', raw_audio)
                samples = [max(-32768, min(32767, s >> 16)) for s in samples_32bit]
            else:
                raise ValueError(f"Unsupported sample width: {sample_width}")
            
            # Convert stereo to mono if needed (mix channels)
            if channels == 2:
                mono_samples = []
                # Ensure we have an even number of samples for stereo processing
                if len(samples) % 2 != 0:
                    samples = samples[:-1]  # Remove last sample if odd

                for i in range(0, len(samples), 2):
                    if i + 1 < len(samples):  # Safety check
                        # Mix left and right channels
                        mixed = (samples[i] + samples[i+1]) // 2
                        mono_samples.append(mixed)
                samples = mono_samples
                channels = 1
                print(f"Converted stereo to mono: {len(samples)} samples")
            
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
        import traceback
        traceback.print_exc()
        return False
    
    return True

if __name__ == "__main__":
    # Convert all drum samples to Mozzi format
    drum_samples = [
        ("source/kick.wav", "src/kick_sample.h", "KICK_SAMPLE"),
        ("source/snare.wav", "src/snare_sample.h", "SNARE_SAMPLE"),
        ("source/high-hat.wav", "src/hihat_sample.h", "HIHAT_SAMPLE"),
        ("source/tom.wav", "src/tom_sample.h", "TOM_SAMPLE")
    ]

    print("Converting drum samples to Mozzi format...")
    print("=" * 50)

    all_success = True

    for input_file, output_file, sample_name in drum_samples:
        print(f"\nConverting {os.path.basename(input_file)}...")

        if not os.path.exists(input_file):
            print(f"Warning: Input file '{input_file}' not found - skipping")
            continue

        # Convert with 2 second maximum for drum samples (shorter for memory efficiency)
        success = convert_wav_to_mozzi_sample(input_file, output_file, max_duration=2.0, sample_name=sample_name)

        if success:
            print(f"✅ {os.path.basename(input_file)} -> {output_file}")
        else:
            print(f"❌ Failed to convert {os.path.basename(input_file)}")
            all_success = False

    print("\n" + "=" * 50)
    if all_success:
        print("🎉 All drum samples converted successfully!")
        print("\nGenerated files:")
        for _, output_file, _ in drum_samples:
            if os.path.exists(output_file):
                print(f"  - {output_file}")
        print("\nNext steps:")
        print("1. Include the sample headers in your main.cpp")
        print("2. Create AudioSample objects for each drum")
        print("3. Update button triggers to use different samples")
    else:
        print("⚠️  Some conversions failed. Check the output above.")
        sys.exit(1)
