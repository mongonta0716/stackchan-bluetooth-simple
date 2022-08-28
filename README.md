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

- [M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar)<br>2022/8/28現在ライブラリの0.8.0では動かないのでGitHubの最新を利用してください。(setSpeechFontが実装されていません。)

- [ServoEasing](https://github.com/ArminJo/ServoEasing)

- [ESP32Servo](https://github.com/madhephaestus/ESP32Servo)

- [M5Unified](https://github.com/m5stack/M5Unified)

- [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio)

- [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP)

# 設定

初期設定はソース内の[設定エリア](https://github.com/mongonta0716/stackchan-bluetooth-simple/blob/main/src/main.cpp#L21-L96)を書き換えてください。

## サーボピン

使用するGPIOの番号を指定します。

- SERVO_PIN_X

- SERVO_PIN_Y

## サーボの初期設定

サーボの90°がズレている際に角度を調整します。±の値を指定します。

- servo_offset_x

- servo_offset_y

## サーボのスピード

待機時とBluetoothスピーカーで音が出ているときの間隔を指定します。(msec)
最小値と最大値で範囲を指定して、ランダムの値を使用します。

- interval_min, interval_max<br>サーボが動くまでの待機時間（無音時）

- interval_move_min, interval_move_max<br>サーボが次の角度に移動する時間（無音時）

- sing_interval_min, sing_interval_max<br>サーボが動くまでの待機時間(Bluetoothスピーカーが鳴っているとき)

- sing_move_min, sing_move_max<br>サーボが次の角度に移動する時間(Bluetoothスピーカーが鳴っているとき)

## Bluetoothの設定

- bt_device_name<br>Bluetooth接続時に表示されるデバイス名を指定します。

- bluetooth_mode<br>trueにすると起動時にBluetoothスピーカーモードになります。<br>falseの場合はBtnAを押すとBluetoothモードに入ります。

## 自動電源OFF時間(Core2のみ)

- auto_power_off_time<br>USBからの電源供給が止まった後、電源OFFするまでの時間。0は自動電源OFFしません。

## セリフの設定

Bluetoothモードで無いときは、下記で設定した言葉を表示します。

- font_name<br>フォントを指定します。使用できるフォントは[こちらのFontList](https://docs.m5stack.com/en/api/m5gfx/m5gfx_appendix)を見てください。

- lyrics<br>設定した文字をランダムに表示します。

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



