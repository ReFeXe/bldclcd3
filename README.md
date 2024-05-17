This fork VESC firmware
https://github.com/vedderb/bldc/tree/release_6_02

additional LCD 3 and high


1. git clone http://github.com/ReFeXe/bldclcd3.git
2. cd bldclcd3
3. git checkout release_6_02

Optional: make arm_sdk_install

4. make ubox_v2_75 



after flashing, change UART baudrate 9600


BY BY UART, LCD3 Active PAS and ADC + PAS, on it PAS pin's change to SWDIO and SWCLK

In UBOX, it is loaded only into the first controller

THIS Firemware is tested and worked, but the LCD functions are being finalized
![bldclcd3](https://user-images.githubusercontent.com/129334095/229014217-7b9d9bf6-c86d-4702-b157-ef4f8fd96065.jpg)









Parametrs KT display 

To begin with, reset the settings to default
С10 - YES

Then you can configure VESC via lcd

C8 = Enable ESC (Motor) Temp

C13 = 0 - ESC Temp, 1 - Motor temp;


P4 = 0 - Fixed throttle (The speed buttons control only the PAS),  1 - The speed buttons control the total power

L1 - PAS Sensor one magnet. (KT one hall sensor). 1 Enable. 0 Disable (Default VESC QEM sensor).

L3 - setting the power to 0 at 0 speed throttle and pas. Default - 1


if you use a reducer (gear) motor wheel, and you need use an external speed sensor. 
This will allow you to track the speed without rotating the motor.
On the reducer motor, e-bike, as a rule, there is a white wire on which there is a hall sensor powered by the main sensors.
To use it, in this firmware it is possible to receive this data using the 'servo' pin.

To compile the firmware with the ability to use an external speed sensor, a string is required.
#define HW_HAS_WHEEL_SPEED_SENSOR
to file lcd3.h

If you are not using an external speed sensor, you need to delete this line.

