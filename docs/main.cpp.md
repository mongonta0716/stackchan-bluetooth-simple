# main.cpp 詳細解説

このドキュメントでは、`src/main.cpp` の役割と主な処理、構成について解説します。

---

## 概要

- スタックチャンを Bluetooth スピーカーとして動作させるサンプルアプリのメインソースです。
- M5Stack シリーズ（Basic/Gray/Go/Fire/Core2 など）で動作し、Bluetooth A2DP による高音質オーディオ再生とサーボ制御によるアバターの動きを組み合わせています。
- [M5Unified](https://github.com/m5stack/M5Unified) の Bluetooth_with_ESP32A2DP サンプルをベースに、サーボや LED 制御、アバター表示など独自機能を追加しています。

---

## 主な構成・機能

### 1. 初期化・セットアップ

- 関連箇所: `setup()` 関数（約 100〜230 行目）
- M5Stack 本体やスピーカー、SD カード、サーボ、アバター（顔表示）、LED などの初期化を行います。
- 設定ファイル（YAML）を SD カードから読み込み、各種パラメータ（サーボピン、音量、LED 設定など）を反映します。
- Bluetooth モードの初期状態や給電方法も設定ファイルに従って決定します。

### 2. Bluetooth スピーカー機能

- 関連箇所: `BluetoothA2DPSink_M5Speaker` の生成・利用（約 60〜100 行目）、`avrc_metadata_callback()`（約 260〜280 行目）
- `BluetoothA2DPSink_M5Speaker` クラスを使い、Bluetooth 経由で音楽を受信・再生します。
- AVRCP メタデータコールバックやイベントコールバックを利用し、再生中の表情やテキスト表示を制御します。

### 3. サーボ・アバター制御

- 関連箇所: `servoLoop()`（約 110〜170 行目）、`lipSync()`（約 180〜250 行目）、`Avatar` 関連の初期化・制御
- サーボモーターでアバターの首振りやうなずき動作を制御します。
- 音声の波形データから FFT を使ってリップシンク（口パク）を実現しています。
- アバターの表情やセリフ表示もランダムや Bluetooth モードに応じて切り替えます。

### 4. LED 制御

- 関連箇所: `level_led()` など LED 制御関数（約 20〜60 行目）、`lipSync()` 内
- M5GoBottom の LED を音量やモードに応じて点灯・グラデーション表示します。
- LED の ON/OFF やレベル表示は FastLED ライブラリで制御しています。

### 5. ボタン操作

- 関連箇所: `loop()` 関数（約 350〜500 行目）
- BtnA: Bluetooth モードの ON/OFF 切り替え
- BtnB: 音量ダウン
- BtnC: 音量アップ

### 6. 電源・バッテリー管理

- 関連箇所: `loop()` 内のバッテリー・電源管理処理（約 400〜500 行目）
- Core2 など一部機種では USB 給電の有無やバッテリー残量を監視し、条件に応じて自動電源 OFF も実装されています。

---

## 主要な関数・タスク

- `setup()`：各種初期化処理
- `loop()`：ボタン入力や定期処理の監視
- `servoLoop()`：サーボ動作の制御（FreeRTOS タスク）
- `lipSync()`：リップシンク処理（FreeRTOS タスク）
- `avatarStart()/avatarStop()`：アバターの描画制御
- `avrc_metadata_callback()`：Bluetooth メタデータ受信時の処理

---

## 特徴・ポイント

- M5Stack シリーズの多様な機種・拡張（GoBottom, TakaoBase 等）に対応
- 設定ファイル（YAML）による柔軟なカスタマイズ
- サーボ・LED・アバター・Bluetooth の複合制御
- FreeRTOS タスクによる並列処理

---

## 参考

- [M5Unified 公式リポジトリ](https://github.com/m5stack/M5Unified)
- [Bluetooth_with_ESP32A2DP サンプル](https://github.com/m5stack/M5Unified/blob/master/examples/Bluetooth_with_ESP32A2DP/Bluetooth_with_ESP32A2DP.ino)

---

何か追加で知りたい点があればご連絡ください。
