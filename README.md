# ESP32 Hanover Flip-dot Sign Controller

![PCB Top Render](https://github.com/ndoo/esp32-hanover-flipdot/raw/master/pcb/esp32-hanover-flipdot.png)

This controller directly controls Hanover Displays Ltd. dot-boards CMOS logic hardware directly through the parallel 14-pin ribbon cable, without the Hanover 7070 or 7080 processor board.

## Design Considerations

This controller's design is based on [Damien Walsh's work on Hanover flip-dot signs](https://damow.net/fun-with-flip-dot-displays/), with only layout changes targeting PCB fabrication and assembly services.

The following considerations went into the PCB schematic and layout:

* SMT components used where possible
* All components are only on the top side of the PCB
* The ESP32-PICO-D4 SIP is used instead of a separate ESP32 module
* PCB size minimized to increase savings with panelization
* All board enable lines are connected to support up to 4 parallel dot-boards
* Generous use of ground stitching vias

## PCBA Notes

### Dot-board Ribbon Cable

J2 is specified as a 2x7 pin 2.54mm pitch shrouded header for IDC cables, so a matching 2x7 pin 2.54mm pitch IDC connector must be retrofitted on the original Hanover dot-board ribbon cable.

The original Hanover cable is a Lumberg Micromodul series MICA 14 connector which mates to a corresponding MICS 14 header. As the Lumberg Micromodul parts are much harder to source (with the bottom line being that they cost more) and requires a custom footprint on the PCB, a jellybean part solution was preferred.

### Bill of Materials

The BOM CSV provided uses [Seeed Studio OPL](https://www.seeedstudio.com/opl.html) SKUs for jellybean parts as I have a personal preference for Seeed Studio's Fusion PCBA service. If using other PCBA services, please substitute the Seeed OPL SKUs accordingly.

## Arduino Sketch

Work in progress, though [Damien's original code](https://github.com/themainframe/esp32-flipdot-controller/tree/master/main) should work fine with some GPIO pin definition changes.

### ESP32 GPIO mapping

* Enable Display 1 - GPIO 15
* Enable Display 2 - GPIO 13
* Enable Display 3 - GPIO 12
* Enable Display 4 - GPIO 14
* Column Advance - GPIO 27
* Reset - GPIO 26
* Set/Unset - GPIO 25
* Row Advance - GPIO 33
* Coil Pulse - GPIO 2
* LED 1 - GPIO 23
* LED 2 - GPIO 19
* LED 3 - GPIO 22
