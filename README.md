# DIY Bench Power Supply Project
Embedded firmware for a STM32 chip to implement a graphical front end on an DIY Bench Power Supply.

## Firmware design
The firmware is designed to handle six distinct tasks. 
1. Coninuously reads voltage and current from the hand-made voltage and current sense module.
2. Outputs a DAC voltage to control an external switching regulator (Ti-LMR33620) that regulates the output voltage of the power supply.
3. Allow a use to set a target voltage and current limit using and encoder and push-button.
4. Show numeric vales and graph displays in real time on an LCD display
5. Support different modes of operation/display (edit mode, graph mode, ADC raw display, summary mode)
6. Run a loop to implement everything smoothly

## General Project Information
