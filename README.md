# IoT Based Gas Leakage Detection with Automatic Shutoff Valve

## Overview
This project is an IoT-based gas leakage detection system designed to improve industrial and domestic safety. The system continuously monitors gas concentration using an MQ-2 gas sensor connected to an ESP32. When gas leakage is detected above the threshold, the ESP32 activates a buzzer and relay, which closes the solenoid valve automatically to stop the gas supply. The gas level and valve status are uploaded to Firebase and displayed on a real-time web dashboard. A thermal camera is also integrated to detect abnormal heat or fire hazards.

## Components Used

- ESP32
- MQ-2 Gas Sensor
- 5V Relay Module
- Solenoid Valve
- Active Buzzer
- Thermal Camera
- Firebase Realtime Database
- HTML
- CSS
- JavaScript

## Features

- Real-time gas leakage detection
- Automatic gas shutoff
- Buzzer alarm
- Firebase cloud monitoring
- Live web dashboard
- Wi-Fi communication using ESP32
- Thermal monitoring for fire detection

## Software Used

- Arduino IDE
- Visual Studio Code
- Firebase

## Project Flow

MQ-2 Gas Sensor
↓

ESP32
↓

Gas Value Analysis
↓

Gas Leak?
↓

Yes
↓

Buzzer ON
↓

Relay ON
↓

Solenoid Valve CLOSED
↓

Upload Data to Firebase
↓

Website Updates in Real Time
↓

Thermal Camera Monitoring

## Author

Albin Paul