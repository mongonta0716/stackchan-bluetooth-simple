# プロジェクト構成

このリポジトリのディレクトリ構成は以下のようになっています：

```
stackchan-bluetooth-simple/
  ├── data/
  │   └── yaml/
  │       └── SC_BasicConfig.yaml
  ├── lib/
  │   └── stackchan-arduino
  ├── LICENSE
  ├── platformio.ini
  ├── README_en.md
  ├── README.md
  ├── src/
  │   ├── BluetoothA2DPSink_M5Speaker.hpp
  │   ├── formatString.hpp
  │   └── main.cpp
```

## 各ディレクトリ・ファイルの概要

- **data/yaml/SC_BasicConfig.yaml**  
  設定ファイルなど、プロジェクトの動作に必要な YAML 形式のデータが格納されています。

- **lib/stackchan-arduino**  
  外部または自作のライブラリが格納されるディレクトリです。`stackchan-arduino`は Stack-chan 関連の Arduino ライブラリと思われます。

- **LICENSE**  
  プロジェクトのライセンス情報が記載されています。

- **platformio.ini**  
  PlatformIO のプロジェクト設定ファイルです。使用するボードやライブラリ、ビルド設定などが記載されています。

- **README.md / README_en.md**  
  プロジェクトの概要や使い方が記載されています。`README.md`は日本語、`README_en.md`は英語版です。

- **src/**  
  ソースコードが格納されています。
  - **BluetoothA2DPSink_M5Speaker.hpp**  
    Bluetooth A2DP（高音質オーディオ伝送）と M5Speaker に関する実装が含まれているヘッダファイルです。
  - **formatString.hpp**  
    文字列整形に関するユーティリティが含まれているヘッダファイルです。
  - **main.cpp**  
    プロジェクトのエントリーポイントとなるメインのソースファイルです。
