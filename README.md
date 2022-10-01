# stackchan-bluetooth-simple

日本語 | [English](README_en.md)

# 概要

M5Stack-AvatarをベースにシンプルにBluetoothスピーカー機能とスタックチャンのサーボコントロール機能をつけました。
[M5Unified](https://github.com/m5stack/M5Unified)のexampleであるBluetooth_with_ESP32A2DPをベースに改造しています。


# 開発環境
- VSCode
- PlatformIO

# 対応機種

- M5Stack Basic/Gray/M5Go<br>BasicはFlashメモリが16MBの機種のみです。

- M5Stack Fire

- M5Stack Core2 / Core2 for AWSIoT

# 必要なライブラリ
Arduino-ESP32は2.0.4(Fireのみ2.0.0)で動作確認しています。M5Stack Fireはarduino-esp32v2.0.4だと不具合があり起動しません。

詳しいバージョンについては[platformio.ini](https://github.com/mongonta0716/stackchan-bluetooth-simple/blob/main/platformio.ini)を見てください。

- [M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar)

- [ServoEasing](https://github.com/ArminJo/ServoEasing)

- [ESP32Servo](https://github.com/madhephaestus/ESP32Servo)

- [M5Unified](https://github.com/m5stack/M5Unified)

- [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio)

- [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP)

- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

# 設定
設定用のJSONファイルがないとデフォルト値を利用します。（Core2のPortCへサーボを接続する設定になっています。）
SDカードに`/json/SC_Config.json`を配置すると自分の設定が利用できます。

初期設定は[JSONファイル](https://github.com/mongonta0716/stackchan-bluetooth-simple/blob/main/data/json/SC_Config.json)を参照してください。


## 設定項目

- servo
    - pin
        - x<br> X軸のGPIOを指定
        - y<br> Y軸のGPIOを指定
    - offset<br>サーボの軸が90°にしたときにズレを修正するパラメータ
        - x<br> X軸のオフセット値を設定
        - y<br> Y軸のオフセット値を設定

    - speed<br>待機時とBluetoothスピーカーで音が出ているときの待機時間とサーボの移動時間を指定します。最小値と最大値で範囲を指定して、ランダムの値を使用します。
        - normal_mode
             - interval_min
             - interval_max
             - move_min
             - move_max
        - sing_mode
             - interval_min
             - interval_max
             - move_min
             - move_max
- bluetooth
    - device_name<br>Bluetoothスピーカーのデバイス名を指定します。
    - starting_state<br>起動時にBluetoothモードにするかどうかを指定します。

- auto_power_off_time<br>Core2のみ。USBの電源供給がOFFになったあと設定した時間が経過すると電源OFFになります。（0は電源OFFしない）

- balloon<br>吹き出しの設定をします。
    - font_language<br>フォントの言語を指定します。"JA"か"CN"、指定しないとラテンフォントを使用します。
    - lyrics<br>ノーマルモード時にランダムで表示するセリフを設定します。最大10個まで。

# 使い方

- BtnA<br>Bluetoothモードに入ります。(bluetooth_mode = falseの時のみ有効)<br>

- BtnB<br>音量を下げます。

- BtnC<br>音量を上げます。

# Credit
- [meganetaaan](https://github.com/meganetaaan)
- [lovyan03](https://github.com/lovyan03/LovyanGFX)
- [robo8080](https://github.com/robo8080)
- [tobozo](https://github.com/tobozo)

# LICENSE
[MIT](LICENSE)

# Author
[Takao Akaki](https://github.com/mongonta0716)



