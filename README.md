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

- [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio)

- [stackchan-arduino](https://github.com/mongonta0716/stackchan-arduino)

# コンパイル時の注意

- M5Stack Fire V2.6/M5Stack Basic V2.6<br>TFカードを使用する際にSD.begin()を20MHz以上では読み込めない事象を確認しました。15MHzに下げています。

- M5Stack Basic V2.6<br>VSCode+PlatformIOでコンパイルするときのenvは`env:m5stack-grey`を選択してください。

# 設定
SDカードに設定用のYAMLファイルがないとデフォルト値を利用します。（PortAへサーボを接続する設定になっています。）
SDカードに`/yaml/SC_BasicConfig.yaml`を配置すると自分の設定が利用できます。

**2022/10/18にJSONからYAMLへ変更しました。JSONからYAMLへのコンバートは[JSON から YAML コンバータ](https://www.site24x7.com/ja/tools/json-to-yaml.html)にて可能です。**<br>コメントの扱いが変わっているので注意してください。
**2025/03/24にstackchan-arduinoライブラリを使用するように変更しました。SC_Config.yamlの内容を元にSC_BasicConfig.yamlを作成してください。**

詳しくは[YAMLファイル](https://github.com/mongonta0716/stackchan-bluetooth-simple/blob/main/data/yaml/SC_BasicConfig.yaml)を参照してください。

## 設定項目
(カッコ内)は初期値
- servo
    - pin
        - x(Core1 22, Core2 33, CoreS3 1)<br> X軸のGPIOを指定(シリアルサーボの場合はRX)
        - y(Core1 21, Core2 32, CoreS3 2)<br> Y軸のGPIOを指定(シリアルサーボの場合はTX)
    - offset<br>サーボの軸が90°にしたときにズレを修正するパラメータ
        - x(0)<br> X軸のオフセット値を設定
        - y(0)<br> Y軸のオフセット値を設定
    - center<br>サーボの中心位置を指定
        - x(180)<br> X軸の中心位置
        - y(270)<br> Y軸の中心位置
    - lower_limit<br>サーボの下限値を指定
        - x(0)<br> X軸の下限値
        - y(240)<br> Y軸の下限値
    - upper_limit<br>サーボの上限値を指定
        - x(360)<br> X軸の上限値
        - y(280)<br> Y軸の上限値
    - speed<br>待機時とBluetoothスピーカーで音が出ているときの待機時間とサーボの移動時間を指定します。最小値と最大値で範囲を指定して、ランダムの値を使用します。
        - normal_mode
             - interval_min(3000)
             - interval_max(6000)
             - move_min(500)
             - move_max(1500)
        - sing_mode
             - interval_min(500)
             - interval_max(1000)
             - move_min(500)
             - move_max(1000)
- bluetooth
    - device_name(M5Stack)<br>Bluetoothスピーカーのデバイス名を指定します。
    - starting_state(false)<br>起動時にBluetoothモードにするかどうかを指定します。
    - start_volume(100)<br>Bluetoothスピーカーの初期値を設定

- auto_power_off_time(0)<br>Core2のみ。USBの電源供給がOFFになったあと設定した時間が経過すると電源OFFになります。（0は電源OFFしない）

- balloon<br>吹き出しの設定をします。
    - font_language("CN")<br>フォントの言語を指定します。"JA"か"CN"、指定しないとラテンフォントを使用します。
    - lyrics<br>ノーマルモード時にランダムで表示するセリフを設定します。最大10個まで。
- led_lr(0)<br>GoBottom1/2で音量に対応してLEDが光るようになります。※ソースの書き換えとコンパイルが必要。
    - 0: ステレオ
    - 1: 左の音量のみに反応
    - 2: 右の音量のみに反応
- led_pin(15)<br>LEDのGPIOピン番号を指定（GoBottom1: 15, GoBottom2: 25）
- takao_base(false)<br>[Stack-chan_Takao_Base](https://ssci.to/8905)を使用するかどうかの設定。(※現在対応しているのはCore2のみ)
- servo_type("DYN_XL330")<br>サーボの種類を指定
    - "PWM": SG90系のPWMサーボ
    - "SCS": Feetech SCS0009 シリアルサーボ
    - "DYN_XL330": Dynamixel XL330 シリアルサーボ
- extend_config_filename("")<br>アプリケーション用の設定ファイル名
- extend_config_filesize(2048)<br>機能拡張用のバッファサイズ
- secret_config_filename("")<br>個人情報用の設定ファイル名
- secret_config_filesize(2048)<br>個人情報用のバッファサイズ
- secret_info_show(true)<br>個人情報をログに出力するかどうか
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



