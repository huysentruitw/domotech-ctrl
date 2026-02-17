[![PlatformIO Test - Native Environment](https://github.com/huysentruitw/domotech-ctrl/actions/workflows/platformio-test.yml/badge.svg)](https://github.com/huysentruitw/domotech-ctrl/actions/workflows/platformio-test.yml)

# Domotech Controller

ESP32-based home automation controller for managing various modules like dimmers, push buttons, and teleruptors through a modular RS-485 bus system. Features WiFi connectivity, Home Assistant integration via MQTT, and a comprehensive filtering system for advanced input processing.

## Features

- **Built on ESP32** using ESP-IDF framework
- **Modular architecture** supporting multiple device types:
  - Dimmer modules for light control with brightness adjustment
  - Push button modules for input handling
  - Push button with temperature sensor modules for climate control
  - Teleruptor modules for toggle control
- **RS-485 bus communication** for reliable device-to-controller communication
- **WiFi connectivity** with station and access point modes
- **Home Assistant integration** via MQTT for seamless home automation
- **Configuration management** through INI files stored in LittleFS
- **Advanced filtering system** for input processing and debouncing
- **Persistent storage** with LittleFS file system support
- **Thread-safe operations** with built-in locking mechanisms
- **Comprehensive unit testing** support for native environment

## Requirements

- PlatformIO
- ESP32 development board (tested with ESP32-DevKit-C)
- Visual Studio Code with PlatformIO extension (recommended)

## Project Structure

- `src/` - Main application source code
  - `main.cpp` - Application entry point with HTTP server
  - `KeyVault.h` - Configuration and credentials management
- `lib/` - Project libraries
  - `Application/` - Core application logic and module management
  - `ApplicationAbstractions/` - Interface definitions
  - `Bus/` - RS-485 bus communication system
  - `Filtering/` - Input filtering and processing implementations
  - `HomeAssistant/` - MQTT integration and device bridge
  - `Ini/` - Configuration file handling (reading/writing)
  - `Locking/` - Thread-safe synchronization primitives
  - `Module/` - Module type implementations
  - `Storage/` - Persistent storage abstraction (LittleFS)
  - `Wifi/` - WiFi connectivity management
- `test/` - Comprehensive unit tests for all major components
- `include/` - Header files
- `partitions/` - ESP32 partition tables

## Building and Testing

### Setup

1. Install [PlatformIO](https://platformio.org/)
2. Clone this repository
3. Open in VS Code with PlatformIO extension

### Building

```bash
# Build for ESP32
pio run -e esp32dev

# Build tests (native environment)
pio run -e native
```

### Testing

```bash
# Run all unit tests
pio test -e native

# Run specific test
pio test -e native -f Application/test_FilterCollection
```

### Flashing

Connect your ESP32 board and run:

```bash
pio run -e esp32dev -t upload

# Monitor serial output
pio run -e esp32dev -t monitor
```

## Configuration

The device automatically enters setup mode in the following scenarios:
- No WiFi credentials are stored in NVS (Non-Volatile Storage)
- Failed to connect to the configured WiFi network

### Setup Mode

When in setup mode:
1. The device starts an **Access Point** with:
   - **SSID**: `DOMOTECH_CTRL`
   - **Password**: `domotech`
2. Connect to this access point and navigate to the configuration web server
3. Use the web interface to:
   - Configure WiFi credentials
   - Set MQTT broker details (URI, username, password)
   - Configure modules, pins, and bus parameters
   - Manage device filters and connections
4. The LED blinks continuously while in setup mode

### Configuration File

Device settings are persisted in NVS (Non-Volatile Storage) and on the device's LittleFS storage:
- WiFi SSID and password
- Home Assistant MQTT connection details (URI, username, password)
- Module configurations (addresses, types, pins)
- Filter definitions and connections
- Bus parameters

After initial setup, the device will automatically connect to the configured WiFi network and Home Assistant MQTT broker on startup.

## Supported Module Types

- **Dimmer**: Variable brightness control (PWM-based)
- **Switch**: On/off binary control
- **Shutter**: Position-based motor control
- **Light**: Combined advanced lighting functionality
- **Input**: Digital input with configurable behavior
- **Push Button**: Momentary switch input
- **Push Button with Temperature**: Input combined with temperature sensor
- **Teleruptor**: Toggle relay control

## License

This project is licensed under the MIT License - see the LICENSE file for details.

