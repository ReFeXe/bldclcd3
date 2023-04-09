This fork VESC firmware
https://github.com/vedderb/bldc/tree/release_6_02

additional LCD 3 and high


1. git clone http://github.com/ReFeXe/bldclcd3.git
2. cd bldclcd3
3. git checkout release_6_02

Optional: make arm_sdk_install

4. make ubox_v2_75 



after flashing, change UART baudrate 9600



THIS Firemware is tested and worked, but the LCD functions are being finalized
![bldclcd3](https://user-images.githubusercontent.com/129334095/229014217-7b9d9bf6-c86d-4702-b157-ef4f8fd96065.jpg)



Parametrs KT display 

To begin with, reset the settings to default
С10 - YES

Then you can configure VESC via lcd

C8 = Enable ESC (Motor) Temp

C13 = 0 - ESC Temp, 1 - Motor temp;


P4 = 0 - Fixed throttle (The speed buttons control only the PAS),  1 - The speed buttons control the total power

