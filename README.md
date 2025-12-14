# DIY Bench Power Supply Project
Embedded firmware for a STM32 chip to implement a graphical front end on an DIY Bench Power Supply.

![Bench Power Supply](./assets/Power%20Supply.jpg)

## Firmware design
The firmware is designed to handle six distinct tasks. 
1. Coninuously reads voltage and current from the hand-made voltage and current sense module.
2. Outputs a DAC voltage to control an external switching regulator (Ti-LMR33620) that regulates the output voltage of the power supply.
3. Allow a use to set a target voltage and current limit using and encoder and push-button.
4. Show numeric vales and graph displays in real time on an LCD display
5. Support different display modes (edit mode, graph mode, ADC raw display, summary mode)
6. Run a loop to implement everything smoothly

![Current Sense Module](./assets/Current%20Sense%20Module.jpg)

## General Project Information
The power supply has two modes of operation, <ins>Constant Voltage Mode</ins> and <ins>Current Limiting Mode</ins>.

### Constant Voltage Mode
The microcontroller will provide a control signal to a switching regulator to maintain the output voltage near the target level set by the user. If the microcontroller detects the current has risen above the current limit set by the user, the supply will automatically shift to <ins>Current Limiting Mode</ins>. 

### Current Limiting Mode
When in Current Limiting Mode, the current signal is lowered to reduce overall output voltage below the user-set voltage goal. This maintains output current at or below the limit set by the user. If the microcontroller detects the current has dropped below the user-set limit (safe), it will raise the output voltage again until either the current limit is reached again or the voltage goal is reached. If the latter happens, the mode switches back to <ins>Constant Voltage Mode</ins>.