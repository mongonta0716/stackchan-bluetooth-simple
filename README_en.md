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

- [M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar)

- [ServoEasing](https://github.com/ArminJo/ServoEasing)

- [ESP32Servo](https://github.com/madhephaestus/ESP32Servo)

- [M5Unified](https://github.com/m5stack/M5Unified)

- [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio)

- [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP)

- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

# Compile Notes

- M5Stack Fire V2.6/M5Stack Basic V2.6<br>When using a TF card, we have confirmed an event that SD.begin() cannot be read at 25MHz. Lower it to 15MHz.

- M5Stack Basic V2.6<br>Please select `env:m5stack-grey` for env when compiling with VSCode+PlatformIO.

# Settings
If there is no JSON file for configuration, default values are used. (It is configured to connect servos to PortC of Core2.)
If you place `/json/SC_Config.json` in SD card, you can use your own settings.

Please refer to [JSON file](https://github.com/mongonta0716/stackchan-bluetooth-simple/blob/main/data/json/SC_Config.json) for the default settings.

Please rewrite [setting area](https://github.com/mongonta0716/stackchan-bluetooth-simple/blob/main/src/main.cpp#L21-L96) in the source for the initial settings.

## Setting Items

(Initial values in parentheses)

- servo
    - pin
        - x(Core1 22, Core2 13)<br> GPIO No of Servo X
        - y(Core1 21, Core2 14)<br> GPIO No of Servo Y
    - offset<br>
        - x(0)<br> Offset x
        - y(0)<br> Offset y

    - speed<br>Specify the standby time and servo travel time when the sound is on standby and Bluetooth speaker. Specify a range with minimum and maximum values and use random values.
        - normal_mode
             - interval_min(5000)
             - interval_max(10000)
             - move_min(500)
             - move_max(1500)
        - sing_mode
             - interval_min(1000)
             - interval_max(2000)
             - move_min(500)
             - move_max(1500)
- bluetooth
    - device_name(M5Stack_BTSPK)<br>Specify the device name of the Bluetooth speaker.
    - starting_state("false")<br>Specify whether Bluetooth mode is enabled or disabled at startup.

- auto_power_off_time(0)<br>Core2 only. power off after USB power supply is turned off and after a set time elapses. (0 does not power off)

- balloon<br>Set up callouts.
    - font_language("JA")<br>Specifies the language of the font." JA" or "CN", otherwise Latin font is used.
    - lyrics("おはよう","Hello","你好")<br>Sets lines to be displayed at random in Normal mode. Up to 10 lines.

# Usage

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



