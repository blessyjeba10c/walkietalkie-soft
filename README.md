# ESP32 DMR Walkie-Talkie System

A comprehensive ESP32-based tactical communication system featuring DMR radio, GPS tracking, GSM fallback, LoRa mesh networking, and Bluetooth control. Built for military/tactical environments with encryption, emergency features, and modular architecture.

## 🎯 Overview

This project implements a complete walkie-talkie system using the ESP32 microcontroller and DMR828S digital radio module. It provides secure, multi-protocol communication with GPS integration, emergency features, and wireless control capabilities.

### Key Features

- **🔊 DMR Digital Radio Communication**: Full-featured DMR walkie-talkie with voice calls, SMS, emergency alerts
- **📍 GPS Location Tracking**: Real-time positioning with NMEA protocol support
- **📱 GSM Fallback SMS**: Emergency SMS when DMR network is unavailable
- **📡 LoRa Mesh Networking**: Long-range, low-power mesh communication
- **🔐 Encryption Support**: Secure communications with AES encryption
- **📞 Bluetooth Control**: Wireless interface for remote operation
- **⚡ Emergency Features**: SOS alerts, emergency GPS broadcasting
- **🔋 Power Management**: Duty cycle modes and power optimization
- **🖥️ Display Integration**: OLED/LCD support for status display

## 🏗️ System Architecture

### Hardware Components

```
┌─────────────────┐    ┌──────────────┐    ┌─────────────┐
│     ESP32       │◄──►│   DMR828S    │◄──►│  Antenna    │
│   Controller    │    │ Radio Module │    │             │
└─────────────────┘    └──────────────┘    └─────────────┘
         │
    ┌────┴────┬─────────┬─────────┬──────────┐
    │         │         │         │          │
┌───▼───┐ ┌──▼──┐  ┌───▼───┐ ┌───▼────┐ ┌──▼──┐
│  GPS  │ │ GSM │  │ LoRa  │ │Display │ │ I/O │
│Module │ │SIM  │  │Module │ │ OLED   │ │Keys │
└───────┘ └─────┘  └───────┘ └────────┘ └─────┘
```

### Software Architecture

```
┌─────────────────────────────────────────────────────────┐
│                 Main Application                        │
│                 (WalkieTalkie.cpp)                     │
├─────────────────────────────────────────────────────────┤
│              Command Processor                          │
│             (CommandProcessor.cpp)                     │
├─────────────────────────────────────────────────────────┤
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────────┐  │
│  │   GSM   │ │   GPS   │ │  LoRa   │ │ Encryption  │  │
│  │Commands │ │Commands │ │Commands │ │  Commands   │  │
│  └─────────┘ └─────────┘ └─────────┘ └─────────────┘  │
├─────────────────────────────────────────────────────────┤
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────────┐  │
│  │   GSM   │ │   GPS   │ │  LoRa   │ │   Display   │  │
│  │Manager  │ │Manager  │ │Manager  │ │   Manager   │  │
│  └─────────┘ └─────────┘ └─────────┘ └─────────────┘  │
├─────────────────────────────────────────────────────────┤
│              DMR828S Library                            │
│         (High-Level + Low-Level APIs)                  │
└─────────────────────────────────────────────────────────┘
```

## 📁 Project Structure

```
walkietalkie-soft/
├── src/                           # Main application code
│   ├── main.cpp                   # Entry point
│   ├── WalkieTalkie.cpp           # Core system logic
│   ├── CommandProcessor.cpp       # Command dispatcher
│   ├── commands/                  # Modular command handlers
│   │   ├── GSMCommands.cpp/h      # GSM operations
│   │   ├── GPSCommands.cpp/h      # GPS operations
│   │   ├── LoRaCommands.cpp/h     # LoRa operations
│   │   └── EncryptionCommands.cpp/h # Encryption
│   └── managers/                  # Hardware managers
│       ├── GSMManager.cpp/h       # GSM module control
│       ├── GPSManager.cpp/h       # GPS module control
│       ├── LoRaManager.cpp/h      # LoRa module control
│       ├── DisplayManager.cpp/h   # Display control
│       └── KeyboardManager.cpp/h  # Input handling
├── include/                       # Header files
│   └── *.h                        # Public interface headers
├── lib/                           # Local libraries
│   └── DMR828S/                   # DMR radio library
│       ├── DMR828S.cpp/h          # High-level API
│       ├── DMR828S_utils.cpp/h    # Low-level protocol
│       ├── COMMANDS.md            # Command reference
│       └── README.md              # Library documentation
├── examples/                      # Example sketches
│   ├── Simple_DMR/                # Basic DMR usage
│   ├── Complete_Walkie_Talkie/    # Full system demo
│   ├── Low_Level_Protocol/        # Protocol access
│   └── DMR_Demo/                  # Advanced features
├── GSM_Test.ino                   # Standalone GSM tester
├── DM828_PROTOCOL.md              # Protocol documentation
├── TODO                           # Development roadmap
└── README.md                      # This file
```

## 🚀 Quick Start

### Prerequisites

1. **Hardware**:
   - ESP32 development board (ESP32-DevKitC recommended)
   - DMR828S digital radio module
   - GPS module (UART/Serial)
   - GSM module (SIM800/SIM900)
   - Optional: LoRa module, OLED display

2. **Software**:
   - [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
   - USB drivers for ESP32

### Installation

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd walkietalkie-soft
   ```

2. **Open in VS Code with PlatformIO**:
   ```bash
   code .
   ```

3. **Install dependencies**:
   PlatformIO will automatically install dependencies listed in `platformio.ini`

4. **Configure hardware pins**:
   Edit `include/WalkieTalkie.h` to match your hardware setup:
   ```cpp
   #define GSM_RX_PIN 25
   #define GSM_TX_PIN 26
   #define I2C_SDA 21
   #define I2C_SCL 22
   ```

### Hardware Connections

| Component | ESP32 Pin | Notes |
|-----------|-----------|--------|
| DMR828S RX | GPIO16 | Serial2 |
| DMR828S TX | GPIO17 | Serial2 |
| GPS RX | GPIO4 | Serial0 (configurable) |
| GPS TX | GPIO2 | Serial0 (configurable) |
| GSM RX | GPIO25 | Serial1 |
| GSM TX | GPIO26 | Serial1 |
| Display SDA | GPIO21 | I2C |
| Display SCL | GPIO22 | I2C |
| LoRa CS | GPIO5 | SPI |
| LoRa RST | GPIO14 | Digital |
| LoRa DIO0 | GPIO27 | Interrupt |

### Building and Uploading

1. **Using PlatformIO VS Code**:
   - Press `Ctrl+Shift+P` → "PlatformIO: Build"
   - Press `Ctrl+Shift+P` → "PlatformIO: Upload"
   - Press `Ctrl+Shift+P` → "PlatformIO: Serial Monitor"

2. **Using PlatformIO CLI**:
   ```bash
   # Build project
   pio run
   
   # Upload to ESP32
   pio run --target upload
   
   # Monitor serial output
   pio device monitor
   ```

### First Run

1. **Power on the system** - ESP32 will initialize all modules
2. **Connect via Bluetooth** - Device name: "LittleBoyz"
3. **Send commands** via Bluetooth terminal or Serial Monitor:
   ```
   help        # Show available commands
   status      # Display system status
   info        # Show device information
   dmr:vol:5   # Set DMR volume to 5
   dmr:ch:3    # Set DMR channel to 3
   ```

## 📚 Command Reference

### DMR Commands
```
dmr:vol:<1-9>           # Set volume (1-9)
dmr:ch:<1-16>           # Set channel (1-16)
dmr:call:<id>           # Start voice call to ID
dmr:sms:<id>:<msg>      # Send SMS to ID
dmr:emerg               # Send emergency alert
dmr:id:<id>             # Set radio ID
dmr:stop                # Stop current call
```

### GPS Commands
```
gps:loc                 # Get current location
gps:track:on            # Enable location tracking
gps:track:off           # Disable location tracking
gps:send:<id>           # Send location via DMR
```

### GSM Commands
```
gsm:sms:<number>:<msg>  # Send SMS via GSM
gsm:call:<number>       # Make GSM call
gsm:status              # Check GSM status
```

### LoRa Commands
```
lora:send:<msg>         # Send LoRa message
lora:freq:<freq>        # Set frequency (MHz)
lora:power:<power>      # Set TX power (2-20dBm)
```

### System Commands
```
help                    # Show all commands
status                  # System status
info                    # Device information
reset                   # Soft reset system
mode:<0-2>              # Change demo mode
```

## 🛠️ Development Guide

### Adding New Features

1. **Create command handler**:
   ```cpp
   // src/commands/MyCommands.cpp
   #include "MyCommands.h"
   
   void handleMyCommand(Stream* stream, String params) {
       // Implementation
   }
   ```

2. **Register in CommandProcessor**:
   ```cpp
   // src/CommandProcessor.cpp
   if (command.startsWith("my:")) {
       handleMyCommand(stream, command.substring(3));
       return;
   }
   ```

3. **Add to managers if needed**:
   Create hardware abstraction in `src/managers/`

### Code Structure Guidelines

- **Modular Design**: Each feature in separate files
- **Hardware Abstraction**: Managers handle hardware specifics
- **Command Pattern**: Commands are stateless functions
- **Error Handling**: Always check return values
- **Documentation**: Comment complex protocol interactions

### Testing

1. **Unit Testing**: Use the provided examples
2. **Hardware Testing**: 
   - `GSM_Test.ino` for GSM module
   - `examples/Simple_DMR/` for DMR basics
   - `examples/Complete_Walkie_Talkie/` for full system

3. **Integration Testing**: Use Bluetooth interface for interactive testing

## 🔐 Security Features

### Encryption
- AES encryption for DMR communications
- Configurable encryption keys
- Encrypted SMS support (planned)

### Emergency Features
- Emergency alert broadcasting
- Automatic GPS location sharing
- GSM fallback for critical messages

## 📡 Communication Protocols

### DMR Protocol
- Digital Mobile Radio (DMR) Tier II
- Time-division duplex operation
- Group and private calls
- SMS messaging support
- Emergency alerting

### LoRa Mesh (Planned)
- Long-range, low-power communication
- Mesh networking capabilities
- Encryption support
- Acknowledge system

## 🔋 Power Management

### Current Features
- Duty cycle mode for DMR module
- Sleep modes for peripheral modules
- Power status monitoring

### Planned Features
- INA169 battery monitoring
- Intelligent power scaling
- Module-specific power control

## 🔧 DMR828S Library Architecture

The project includes a comprehensive DMR828S library with dual-layer architecture:

### High-Level API (`DMR828S` class)
- **Purpose**: User-friendly walkie-talkie functions
- **Features**: 
  - Voice calling (private/group/all call)
  - SMS messaging with callbacks
  - Emergency alerts
  - Audio control (volume, mic gain)
  - Channel management
  - Encryption support
  - Event callbacks for received data

### Low-Level API (`DMR828S_Utils` class)  
- **Purpose**: Direct protocol access
- **Features**:
  - Raw frame sending/receiving
  - Manual checksum control
  - Debug frame inspection
  - Protocol-level error handling

### Usage Examples

**Basic DMR Operations**:
```cpp
#include "DMR828S.h"
DMR828S dmr(Serial2);

void setup() {
    dmr.begin(57600);
    dmr.setRadioID(0x123456);
    dmr.setChannel(1);
    dmr.setVolume(5);
}

void loop() {
    // Send SMS
    dmr.sendSMS(0x789ABC, "Hello World!");
    
    // Start voice call
    dmr.startCall(CALL_PRIVATE, 0x789ABC);
    
    // Process events
    dmr.update();
}
```

**Low-Level Protocol Access**:
```cpp
// Access low-level utilities
DMR828S_Utils& lowLevel = dmr.getLowLevel();
uint8_t data[4] = {0x01, 0x02, 0x03, 0x04};
lowLevel.sendFrame(0x24, 0x01, 0x01, data, 4);
```

## 📋 Feature Implementation Status

### Core Radio (✅ Complete)
- [x] Channel selection (1-16)
- [x] Volume control (1-9) 
- [x] Status monitoring
- [x] RSSI reading

### Voice Calling (✅ Complete)
- [x] Private calls
- [x] Group calls
- [x] All call broadcast
- [x] Call status tracking

### SMS Messaging (✅ Complete)
- [x] Send SMS to individuals
- [x] Send SMS to groups
- [x] SMS receive callbacks
- [x] Delivery status tracking

### Emergency Features (✅ Complete)
- [x] Emergency alarm broadcasting
- [x] Emergency callbacks
- [x] GPS integration for alerts

### Audio Control (✅ Complete)
- [x] Microphone gain adjustment
- [x] Speaker volume control
- [x] Audio quality settings

### Advanced Features (🔄 Partial)
- [x] Encryption key management
- [x] Duty cycle power modes
- [ ] Repeater mode configuration
- [ ] Frequency hopping
- [ ] Voice encryption

## 🐛 Troubleshooting

### Common Issues

**Build Errors**:
```bash
# Clean build
pio run --target clean
pio run
```

**Upload Failed**:
- Hold BOOT button during upload
- Check USB cable and port
- Verify ESP32 drivers installed

**Module Not Responding**:
- Check wiring connections
- Verify baud rates in code
- Test with individual examples

**Bluetooth Connection Issues**:
- Clear paired devices
- Reset ESP32
- Check device name in logs

### Debug Information

Enable debug output in `platformio.ini`:
```ini
build_flags = 
    -DCORE_DEBUG_LEVEL=5
    -DDMR_DEBUG=1
```

## 📝 TODO & Roadmap

### High Priority
- [ ] LoRa encryption implementation
- [ ] Mesh networking protocols  
- [ ] Battery monitoring integration
- [ ] Power optimization

### Medium Priority
- [ ] Frequency hopping for anti-jamming
- [ ] Satellite communication (Satcom)
- [ ] Advanced encryption features
- [ ] Web interface

### Low Priority  
- [ ] Mobile app development
- [ ] Cloud integration
- [ ] Advanced GPS features
- [ ] Voice encryption

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make changes following code guidelines
4. Test thoroughly with hardware
5. Submit pull request with detailed description

### Code Style
- Follow existing naming conventions
- Comment complex algorithms
- Use meaningful variable names
- Include error handling
- Document public APIs

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🔗 References

- [DMR828S Protocol Documentation](DM828_PROTOCOL.md)
- [DMR828S Library Reference](lib/DMR828S/README.md)
- [Command Reference](lib/DMR828S/COMMANDS.md)
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [PlatformIO Documentation](https://docs.platformio.org/)

## 📞 Support

For technical support or questions:
- Create an issue in the repository
- Check existing documentation
- Review example code
- Test with provided examples

---

**Note**: This is a tactical communication system designed for professional use. Ensure compliance with local radio regulations and licensing requirements when using DMR frequencies.