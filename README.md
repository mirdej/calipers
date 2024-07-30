# Calipers

Bluetooth extension for cheap digital calipers.
An ESP32 reads data from the caliper serial port and "types" it as a BLE keyboard directly into the host computer.

## Arduino Library
The caliper-reading functions have been condensed into a simple class *Calipers* inside the file **Calipers.h**.
You can use the library as a standalone library for any Arduino (not only ESP32). 
        
        firmware/lib/Calipers

There are different scales and calipers out there with different protocols. See the readme of the library for more information.


