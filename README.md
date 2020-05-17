# ESP32 Hanover Flip-dot Sign Controller

## PCB

Reference PCB is available at [ndoo/esp32-hanover-flipdot](https://github.com/ndoo/esp32-hanover-flipdot)

## Firmware

### IDE Prerequisites

You will need a working [VS Code](https://code.visualstudio.com/) and [PlatformIO](https://platformio.org/) development environment, with ESP32 support installed.

Libraries should automatically download as they have been specified in `platformio.ini`.

### First Use

#### Powering the Board

A freshly assembled board will not have any sketch running on it, and will be in bootloader bode waiting for firmware upload over serial. You will need a TTL USB-serial adapter, and one of the following methods of powering the board:

* Connect Tx, Rx, VCC and GND to USB-serial adapter - **USB-serial adapter must be set to 3.3V VCC or the ESP32 SIP will be irreversibly damaged**
* Connect Tx, Rx and GND to USB-serial adapter, then connect 18VDC power supply - **ground PCB to USB-serial adapter before connecting DC supply to avoid ESD**

**Do not connect RS-232 level signalling to the board, the ESP32 SIP will be damaged by signalling above TTL levels.**

#### Compiling and Uploading Sketch

The default `platformio.ini` provided here is configured to OTA upload for your convenience, however that means you need to edit this file for the initial flash, in order to program over USB-serial, as the board is not running firmware capable of OTA updates.

Comment these 2 lines out by preceding them with a semicolon (`;`):

```
upload_protocol = espota
upload_port = Hanover-Flipdot.local
```

Then, test the build by clicking the tick in the lower left status bar of VS Code + PlatformIO.

If the build succeeds, click the rightwards-pointing arrow in the lower left status bar of VS Code + PlatformIO. This will link the firmware and attempt to upload it to the board's ESP32 over USB-serial.

#### Configuring Wi-Fi

You should see a new wireless AP with the SSID `ESP_<YOUR_ESP_UNIQUE_MAC>`. Connect to it and you should be automatically redirected to a portal that allows you to connect the ESP32 to your Wi-Fi.

If the connection is successful, you should be able to send data to the board using the Python scripts found in the `demos` folder. If the connection fails, the ESP32 will automatically time out and re-start the temporary wireless AP for you to continue Wi-Fi setup.

Don't forget to uncomment the previously commented-out lines in `platformio.ini` to re-enable OTA uploads.

#### Rescue Programming

If you need to re-upload a sketch over serial, take note that this board does not have automatic boot and reset circuitry, so you will need to press the two buttons on the board in the following sequence to enter download mode, before uploading a sketch:

1. Hold down RST button
2. Hold down PGM button
3. Release RST button
4. Release PGM button

This may take a few tries to get the timing right, as the buttons are tiny and placed close together.

#### Troubleshooting

* Board not receiving UDP packets
  * The board, by default, listends on multicast IP 239.1.2.3 on UDP port 8080 (port can be changed in `include/main.h`)
  * Your LAN routers, switches or Wi-Fi access points may be configured to drop multicast packets, or may need IGMP snooping to be enabled
* OTA not working on Windows due to Hanover-Flipdot.local not resolving
  * You may need to install an mDNS resolver software such as [Apple's Bonjour Print Services for Windows](https://support.apple.com/kb/DL999)
