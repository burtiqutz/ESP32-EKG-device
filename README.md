# ESP32 EKG device

 This is a simple implementation of an ECG using an ESP32 for a small university project. The project does not make use of IOT features. There may be issues with the ECG reading as I was not able to test it properly due to issues with the electrodes.

## Components used
- ESP32 DevKit-C
- AD8232 ECG Sensor (kit)
- 3 electrodes (included in kit)
- 3.5mm jack with connections for the electrodes (included in kit)
- SSD1306 Monochrome OLED screen
- Breadboard & wires
- 2 resistors

## Wiring scheme (courtesy of paint)
![image](https://github.com/user-attachments/assets/fac2cb9c-afb0-4582-a4c0-b6af8bccb667)


## Extra notes
Electrodes are meant for single use, so I highly recommend buying multiple pairs or making your own, as reusing them will significantly impact their accuracy.
The electrodes have to be placed according to Einthoven's triangle.
I used a voltage divider on the output of the AD8232.
