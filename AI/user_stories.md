# User Stories for AI (Augment Code Agent)

## Project Description

This is a RP2040-based eurorack module that can play samples at different rates. Initially it will be primarily a multiple voice drum machine that is triggered by trigger signals from another module. If feasable, the module will play 4 different samples (kick, snare, hihat, tom). Each sample should be tuneable and hopefully playable at the same time.

## Project Stories

- [x] As a developer, I would like to change the name of this project to something that is more inline with what it is. I am thinking something like "Pico / DAC Sampler".
- [x] As a developer I would like to make sure that the hardware is working correctly and that I can hear I2S audio created by the RP2040 and sent to the PCM5102A DAC.
- [x] As a developer I would like to use the Mozzi audio library to generate a sine wave and output it via I2S.
- [x] As a developer I would like to be able to play back a WAV file from flash memory using the Mozzi library.
- [x] As a developer I would like to add a OLED screen to the module- https://funkfinger.github.io/binkey-data/parts/frienda-oled-091-i2c-ssd1306

## Module Functionality

- [ ] As a user I would like to be able to choose the sample to play.
- [ ] As a user I would like to be able to trigger the sample to play.
- [ ] As a user I would like to be able to adjust the playback speed of the sample.
- [ ] As a user I would like to be able to adjust the playback pitch of the sample.
