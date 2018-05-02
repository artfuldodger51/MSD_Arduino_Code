# Description

Code to control an Arduino Mega for the final year Mechatronics
System Design project. The Arduino will recieve a control signal 
to send to an ADC to select the speed of a motor, and will
send a 16 bit value back to a PC containing the count value
of a decoder chip.


## Signals recieved 
- startByte: 255
- commandByte: 
    - 0 - Reset HTCL chip
    - 1 - Output to ADC
- ADCByte: ADC Value
- checkSum: Sum of previous values

## Signals sent
- startByte: 255
- lowByte: 8 LSB of decoder
- highByte: 8 MSB of decoder
- checkSum: sum of previous values

