# stackchan-bluetooth-simple

[日本語](README.md) | English

# Overview 

Based on the M5Stack-Avatar, a simple Bluetooth speaker function and servo control function for the stack chan are added.
It is modified based on Bluetooth_with_ESP32A2DP, which is an example of [M5Unified](https://github.com/m5stack/M5Unified).


# environment

- VSCode

- PlatformIO

# Support Devices

- M5Stack Basic/Gray/M5Go<br>Basic is only available with 16MB of Flash memory.

- M5Stack Fire

- M5Stack Core2 / Core2 for AWSIoT

# Requirement

Arduino-ESP32 is confirmed to work with 2.0.4 (Fire only 2.0.0). M5Stack Fire has a problem with arduino-esp32v2.0.4 and does not start.

See [platformio.ini](https://github.com/mongonta0716/stackchan-bluetooth-simple/blob/main/platformio.ini) for detailed version information.

- [M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar)<br>As of 2022/8/28, the library 0.8.0 does not work, so please use the latest GitHub version. (setSpeechFont is not implemented.)

- [ServoEasing](https://github.com/ArminJo/ServoEasing)

- [ESP32Servo](https://github.com/madhephaestus/ESP32Servo)

- [M5Unified](https://github.com/m5stack/M5Unified)

- [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio)

- [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP)

# Settings

Please rewrite [setting area](https://github.com/mongonta0716/stackchan-bluetooth-simple/blob/main/src/main.cpp#L21-L96) in the source for the initial settings.

## Servo PIN

Specify the GPIO to be used.

- SERVO_PIN_X

- SERVO_PIN_Y

## Servo Initial Settings

Adjusts the angle when the servo is out of alignment by 90°. Specify ± value.

- servo_offset_x

- servo_offset_y

## Servo Speed 

Specify the interval between standby and when sound is emitted by the Bluetooth speaker. (msec)
Specify a range with a minimum and maximum value and use a random value.

- interval_min, interval_max<br>Waiting time until servo moves (when silent)

- interval_move_min, interval_move_max<br>Time for the servo to move to the next angle (in silence)

- sing_interval_min, sing_interval_max<br>Waiting time for servo to move (when Bluetooth speaker is sounding)

- sing_move_min, sing_move_max<br>サTime for the servo to move to the next angle (when the Bluetooth speaker is sounding)

## Bluetooth Settings

- bt_device_name<br>bluetooth device name

- bluetooth_mode<br>If true, it will enter Bluetooth speaker mode at startup; if false, pressing BtnA will enter Bluetooth mode.

## Auto Power Off(Only Core2)

- auto_power_off_time<br>Time until power is turned off after power supply from USB stops. 0 means no automatic power off.


## Serif Settings

When not in Bluetooth mode, the words set below are displayed.

- font_name<br>Specify the font. See [here FontList](https://docs.m5stack.com/en/api/m5gfx/m5gfx_appendix) for available fonts.

- lyrics<br>Randomly displays the set characters.

# 使い方

- BtnA<br>Enters Bluetooth mode. (Only valid when bluetooth_mode = false)

- BtnB<br>Volume down.

- BtnC<br>Volume up.

# Credit
- [meganetaaan](https://github.com/meganetaaan)
- [lovyan03](https://github.com/lovyan03/LovyanGFX)
- [robo8080](https://github.com/robo8080)
- [tobozo](https://github.com/tobozo)

# LICENSE
[MIT](LICENSE)

# Author
[Takao Akaki](https://github.com/mongonta0716)



