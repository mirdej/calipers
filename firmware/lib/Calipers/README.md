# Caliper Library

This library lets you read the serial data from some cheap chinese digital calipers.
There are several different protocols out there, so your mileage may vary but this library works on the protocol described here:
https://www.shumatech.com/support/chinese_scales.htm

## Electrical connections

You cannot connect the serial port of the caliper directly to an Arduino - logic high of the caliper data port is only 1V and will not be recognised. You'll need to implement some basic level shifting/logic converter. This library assumes you'll use a NPN transistor and pullups to VCC and thus that your logic levels are inverted. See:

https://www.instructables.com/Hacked-Digital-Vernier-Caliper-Using-Arduino/


## Class methods
    void begin(int datapin, int clockpin);
    bool available();
    bool is_on();
    void print();
    float get_mm();
    float get_inch();