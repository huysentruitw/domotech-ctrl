[![PlatformIO Test - Native Environment](https://github.com/huysentruitw/domotech-ctrl/actions/workflows/platformio-test.yml/badge.svg)](https://github.com/huysentruitw/domotech-ctrl/actions/workflows/platformio-test.yml)

# Domotech Controller

ESP32-based home automation controller for managing various modules like dimmers, push buttons, and teleruptors through a modular bus system.

## Features

- Built on ESP32 using ESP-IDF framework
- Modular architecture for different types of devices:
  - Dimmer modules for light control
  - Push button modules for input handling
  - Teleruptor modules for toggle control
- RS-485 bus-based communication system for reliable device communication
- Configuration management through INI files
- Built-in filtering system for input processing
- Unit testing support for native environment

## Requirements

- PlatformIO
- ESP32 development board
- Visual Studio Code (recommended)

## Project Structure

- `src/` - Main application source code
- `lib/` - Project libraries
  - `Application/` - Core application logic
  - `ApplicationAbstractions/` - Interface definitions
  - `Bus/` - Bus communication system
  - `Filtering/` - Input filtering implementations
  - `Ini/` - Configuration file handling
  - `Module/` - Module implementations
- `test/` - Unit tests
- `include/` - Header files
- `partitions/` - ESP32 partition tables

## Building and Testing

### Setup

1. Install PlatformIO
2. Clone this repository
3. Open in VS Code with PlatformIO extension

### Building

```bash
# Build for ESP32
pio run -e esp32dev

# Build tests
pio run -e native
```

### Testing

```bash
# Run unit tests
pio test -e native
```

### Flashing

Connect your ESP32 board and run:

```bash
pio run -e esp32dev -t upload
```

## Configuration

1. Create a `secrets.h` file in the `src` directory with your WiFi credentials:
   ```cpp
   #pragma once

   #define WIFI_SSID "SomeSsid"
   #define WIFI_PASS "SomePass"
   ```
2. Adjust module addresses and pin configurations in the main application

## License

This project is licensed under the MIT License - see the LICENSE file for details.

