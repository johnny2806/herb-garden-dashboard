# Herb Garden Security Hub (IoT Dashboard)

An industrial-grade IoT monitoring system featuring modular architecture and high-fidelity instrumentation.

## Key Features

- **Modular JavaScript Architecture**: Separated concerns via API, UI, and App modules.
- **Advanced Instrumentation**: Custom-built gauge charts with numerical tick calibration.
- **Cryptographic Security**: Supports ChaCha20_AEAD decrypted payload ingestion.
- **Responsive Layout**: Native CSS grid system for real-time telemetry monitoring.

## Tech Stack

- **Frontend**: HTML5, Native CSS3, JavaScript (ES6 Modules).
- **Visualization**: Chart.js with custom plugins.
- **Backend (Required)**: FastAPI + UDP Ingestion Server.

## Hardware Components (Bill of Materials)

To replicate this setup, the following hardware was utilized:

- **Microcontroller**: Raspberry Pi Pico W (handling encrypted UDP transmission).
- **Sensors**:
  - **DHT22/DHT11**: High-accuracy ambient temperature and humidity sensing.
  - **Capacitive Soil Moisture Sensor**: For real-time saturation monitoring (Analog).
- **Connectivity**: 802.11n Wi-Fi with randomized MAC address support (Stealth Mode).
- **Actuators (Planned)**: 5V Relay module for automated irrigation control.
