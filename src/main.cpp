// スタックチャンをBluetoothスピーカーとして使うサンプルアプリです。
// https://github.com/m5stack/M5Unified のBluetooth_with_ESP32A2DP.inoをベースにサーボの動きを追加しています。
// Copyright (c) 2022 Takao Akaki

#include <Arduino.h>

#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>
#include "Stackchan_servo.h"
#include "BluetoothA2DPSink_M5Speaker.hpp"
#include "Avatar.h"

using namespace m5avatar;
Avatar avatar;
StackchanSERVO servo;

//-------------------------------------------------------------------- Start Of User Setting area ---------------------------------------------------------------------
//-------------------------------------------------------------------- Start Of User Setting area ---------------------------------------------------------------------
//-------------------------------------------------------------------- Start Of User Setting area ---------------------------------------------------------------------
// --------------------
// サーボピンの初期設定(ServoPin setting area)
#if defined(ARDUINO_M5STACK_Core2)
  // M5Stack Core2用のサーボの設定(for M5Stack Core2)
  // Port.A X:G33, Y:G32
  // Port.C X:G13, Y:G14
  // スタックチャン基板 X:G27, Y:G19
  #define SERVO_PIN_X 13
  #define SERVO_PIN_Y 14
#elif defined( ARDUINO_M5STACK_FIRE )
  // M5Stack Fire用のサーボの設定(for M5Stack Fire)
  // M5Stack Fireの場合はPort.A(X:G22, Y:G21)のみです。
  // I2Cと同時利用は不可
  // PortC はPSRAMと競合しています
  #define SERVO_PIN_X 22
  #define SERVO_PIN_Y 21
#elif defined( ARDUINO_M5Stack_Core_ESP32 )
  // M5Stack Basic/Gray/Go用の設定(for M5Stack Basic or Gray or M5Go)
  // Port.A X:G22, Y:G21
  // Port.C X:G16, Y:G17
  // スタックチャン基板 X:G5, Y:G2
  #define SERVO_PIN_X 16
  #define SERVO_PIN_Y 17
#endif
// サーボピンの初期設定end
// --------------------



// --------------------
// サーボ関連の初期設定
#define START_DEGREE_VALUE_X 90         // Xサーボの初期位置（変更しないでください。） Start angle of ServoX
#define START_DEGREE_VALUE_Y 90         // Yサーボの初期位置（変更しないでください。） Start angle of ServoY
int servo_offset_x = 0;                 // X軸サーボのオフセット（90°からの+-で設定）  Offset of ServoX
int servo_offset_y = 0;                 // Y軸サーボのオフセット（90°からの+-で設定）  Offset of ServoY

// ----- あまり間隔を短くしすぎるとサーボが壊れやすくなるので注意(単位:msec)　If you move the servo too often, it will break.
static long interval_min      = 3000;        // 待機時インターバル最小            Standby mode interval MIN
static long interval_max      = 6000;        // 待機時インターバル最大            Standby mode interval MAX
static long interval_move_min = 500;         // 待機時のサーボ移動時間最小        Standby mode move time MIN
static long interval_move_max = 1500;        // 待機時のサーボ移動時間最大        Standby mode move time MAX
static long sing_interval_min = 500;         // 歌うモードのインターバル最小      Sing mode interval MIN
static long sing_interval_max = 1000;        // 歌うモードのインターバル最大      Sing mode interval MAX
static long sing_move_min     = 500;         // 歌うモードのサーボ移動時間最小    Sing mode move time MIN
static long sing_move_max     = 1000;        // 歌うモードのサーボ移動時間最大    Sing mode move time MAX
// サーボ関連の設定 end
// --------------------


// --------------------
// Bluetoothのデバイス名
/// set ESP32-A2DP device name
static constexpr char bt_device_name[] = "ESP32DeviceName";

// 起動時にBluetoothモードにするかどうか
// Flag whether BluetoothMode is enabled or disabled at startup
bool bluetooth_mode = false;
// --------------------

// auto poweroff 
// Time (msec) until power is automatically turned off after USB power supply stops. 0 means power is not turned off.
uint32_t auto_power_off_time = 0;  // USB給電が止まった後自動で電源OFFするまでの時間（msec）。0は電源OFFしない。

// ---------------
// ランダムでセリフをしゃべる。
// フォントによっては4MBのM5StackではFlashメモリが足りなくなる場合があります。
// Some fonts cannot be written in M5Stack with 4MB of Flash because of the size of the sketch.
const lgfx::IFont* font_name = &fonts::efontJA_16;
const char* lyrics[] = {"Hello", "こんにちは", "你好", "Bonjour"};

//---------------------------------------------------------------------- End Of User Setting area ---------------------------------------------------------------------
//---------------------------------------------------------------------- End Of User Setting area ---------------------------------------------------------------------
//---------------------------------------------------------------------- End Of User Setting area ---------------------------------------------------------------------

// --------------------
// Avatar関連の初期設定
#define LIPSYNC_LEVEL_MAX 10.0f
static float lipsync_level_max = LIPSYNC_LEVEL_MAX; // リップシンクの上限初期値
float mouth_ratio = 0.0f;
bool sing_happy = true;
// Avatar関連の設定 end
// --------------------

uint32_t last_discharge_time = 0;  // USB給電が止まったときの時間(msec)

/// set M5Speaker virtual channel (0-7)
static constexpr uint8_t m5spk_virtual_channel = 0;

static BluetoothA2DPSink_M5Speaker a2dp_sink = { &M5.Speaker, m5spk_virtual_channel };
static fft_t fft;
static constexpr size_t WAVE_SIZE = 320;
static int16_t raw_data[WAVE_SIZE * 2];

void servoLoop(void *args) {
  long move_time = 0;
  long interval_time = 0;
  long move_x = 0;
  long move_y = 0;
  float gaze_x = 0.0f;
  float gaze_y = 0.0f;
  bool sing_mode = false;
  for (;;) {
    if (mouth_ratio == 0.0f) {
      // 待機時の動き
      interval_time = random(interval_min, interval_max);
      move_time = random(interval_move_min, interval_move_max);
      lipsync_level_max = LIPSYNC_LEVEL_MAX; // リップシンク上限の初期化
      sing_mode = false;

    } else {
      // 歌うモードの動き
      interval_time = random(sing_interval_min, sing_interval_max);
      move_time = random(sing_move_min, sing_move_max);
      sing_mode = true;
    } 
    avatar.getGaze(&gaze_y, &gaze_x);
    
//    Serial.printf("x:%f:y:%f\n", gaze_x, gaze_y);
    // X軸は90°から+-で左右にスイング
    if (gaze_x < 0) {
      move_x = START_DEGREE_VALUE_X - mouth_ratio * 15 + (int)(30.0 * gaze_x);
    } else {
      move_x = START_DEGREE_VALUE_X + mouth_ratio * 15 + (int)(30.0 * gaze_x);
    }
    // Y軸は90°から上にスイング（最大35°）
    move_y = START_DEGREE_VALUE_Y - mouth_ratio * 10 - abs(25.0 * gaze_y);
    servo.moveXY(move_x, move_y, move_time);
    if (!bluetooth_mode) {
      int exp = random(4);
      int exp2 = random(2);
      avatar.setMouthOpenRatio(1.0f);
      avatar.setSpeechText(lyrics[exp]);
      if (exp2 % 2) {
        avatar.setExpression(Expression::Neutral);
      } else {
        avatar.setExpression(Expression::Happy);
      }
      avatar.setMouthOpenRatio(0.5f);
    }
    //avatar.setExpression((Expression)exp);
    if (sing_mode) {
      // 歌っているときはうなずく
      servo.moveXY(move_x, move_y + 10, 400);
    }
    vTaskDelay(interval_time/portTICK_PERIOD_MS);

  }
}


void lipSync(void *args)
{
  DriveContext *ctx = (DriveContext *)args;
  Avatar *avatar = ctx->getAvatar();
  for (;;)
  {
    uint64_t level = 0;
    auto buf = a2dp_sink.getBuffer();
    if (buf) {
      memcpy(raw_data, buf, WAVE_SIZE * 2 * sizeof(int16_t));
      fft.exec(raw_data);
      for (size_t bx = 5; bx <= 60; ++bx) { // リップシンクで抽出する範囲はここで指定(低音)0〜64（高音）
        int32_t f = fft.get(bx);
        level += abs(f);
        //Serial.printf("bx:%d, f:%d\n", bx, f) ;
      }
      //Serial.printf("level:%d\n", level >> 16);
    }

    // スレッド内でログを出そうとすると不具合が起きる場合があります。
    //Serial.printf("data=%d\n\r", level >> 16);
    mouth_ratio = (float)(level >> 16)/lipsync_level_max;
    if (mouth_ratio > 1.2f) {
      if (mouth_ratio > 1.5f) {
        lipsync_level_max += 10.0f; // リップシンク上限を大幅に超えるごとに上限を上げていく。
      }
      mouth_ratio = 1.2f;
    }
    avatar->setMouthOpenRatio(mouth_ratio);
    vTaskDelay(30/portTICK_PERIOD_MS);
  }
}

void hvt_event_callback(int avatar_expression, const char* text) {
  avatar.setExpression((Expression)avatar_expression);
  avatar.setSpeechText(text);
}

void avrc_metadata_callback(uint8_t data1, const uint8_t *data2)
{
  Serial.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", data1, data2);
  if (sing_happy) {
    avatar.setExpression(Expression::Happy);
  } else {
    avatar.setExpression(Expression::Neutral);
  }
  sing_happy = !sing_happy;

}


void setup(void)
{
  auto cfg = M5.config();

  cfg.external_spk = true;    /// use external speaker (SPK HAT / ATOMIC SPK)
//cfg.external_spk_detail.omit_atomic_spk = true; // exclude ATOMIC SPK
//cfg.external_spk_detail.omit_spk_hat    = true; // exclude SPK HAT

  M5.begin(cfg);


  { /// custom setting
    auto spk_cfg = M5.Speaker.config();
    /// Increasing the sample_rate will improve the sound quality instead of increasing the CPU load.
    spk_cfg.sample_rate = 96000; // default:64000 (64kHz)  e.g. 48000 , 50000 , 80000 , 96000 , 100000 , 128000 , 144000 , 192000 , 200000
    spk_cfg.task_pinned_core = APP_CPU_NUM;
    // spk_cfg.task_priority = configMAX_PRIORITIES - 2;
    spk_cfg.dma_buf_count = 20;
    //spk_cfg.stereo = true;
    // spk_cfg.dma_buf_len = 512;
    M5.Speaker.config(spk_cfg);
  }


  M5.Speaker.begin();
  M5.Speaker.setVolume(200);
  M5.Speaker.setChannelVolume(200, m5spk_virtual_channel);
  
  servo.begin(SERVO_PIN_X, START_DEGREE_VALUE_X, servo_offset_x,
              SERVO_PIN_Y, START_DEGREE_VALUE_Y, servo_offset_y);
  delay(3000);
  avatar.init(); // start drawing
 
  avatar.addTask(lipSync, "lipSync");
  avatar.addTask(servoLoop, "servoLoop");
  avatar.setExpression(Expression::Neutral);
  avatar.setSpeechFont(font_name);

  if (bluetooth_mode) {
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.setHvtEventCallback(hvt_event_callback);
    a2dp_sink.start(bt_device_name, true);
    avatar.setExpression(Expression::Sad);
    avatar.setSpeechText("Bluetooth Mode");
  }

}

void loop(void)
{

  {
    static int prev_frame;
    int frame;
    do
    {
      vTaskDelay(1);
    } while (prev_frame == (frame = millis() >> 3)); /// 8 msec cycle wait
    prev_frame = frame;
  }

  M5.update();
  if (M5.BtnA.wasDeciedClickCount())
  {
    switch (M5.BtnA.getClickCount())
    {
    case 1:
      if (!bluetooth_mode) {
        a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
        a2dp_sink.setHvtEventCallback(hvt_event_callback);
        a2dp_sink.start(bt_device_name, true);
        avatar.setExpression(Expression::Sad);
        avatar.setSpeechText("Bluetooth Mode");
        M5.Speaker.tone(1000, 100);
        bluetooth_mode = true;
      }
      break;

    case 2:
      if (bluetooth_mode) {
        avatar.setExpression(Expression::Neutral);
        avatar.setSpeechText("Normal Mode");
        M5.Speaker.tone(800, 100);
        a2dp_sink.stop();
        a2dp_sink.end(true);
        delay(1000);
        avatar.setSpeechText("");
        bluetooth_mode = false;
      }
      break;
    }
  }
  if (M5.BtnB.wasPressed()) {
    uint8_t volume = M5.Speaker.getChannelVolume(m5spk_virtual_channel);
    volume = volume - 10;
    M5.Speaker.setChannelVolume(m5spk_virtual_channel, volume);
    M5.Speaker.tone(2000, 100);
    delay(200);
    M5.Speaker.tone(1000, 100);
  }
  if (M5.BtnC.wasPressed()) {
    uint8_t volume = M5.Speaker.getChannelVolume(m5spk_virtual_channel);
    volume = volume + 10;
    M5.Speaker.setChannelVolume(m5spk_virtual_channel, volume);
    M5.Speaker.tone(1000, 100);
    delay(200);
    M5.Speaker.tone(2000, 100);
  }
#if not(defined(ARDUINO_M5STACK_FIRE) || defined(ARDUINO_M5Stack_Core_ESP32)) // FireはAxp192ではないのとI2Cが使えないので制御できません。
  if (M5.Power.Axp192.getACINVolatge() < 3.0f) {
    // USBからの給電が停止したとき
    // Serial.println("USBPowerUnPlugged.");
    M5.Power.setLed(0);
    if ((auto_power_off_time > 0) and (last_discharge_time == 0)) {
      last_discharge_time = millis();
    } else if ((auto_power_off_time > 0) and ((millis() - last_discharge_time) > auto_power_off_time)) {
      M5.Power.powerOff();
    }
  } else {
    //Serial.println("USBPowerPlugged.");
    M5.Power.setLed(80);
    if (last_discharge_time > 0) {
      last_discharge_time = 0;
    }
  }
#endif

}

#if !defined ( ARDUINO )
extern "C" {
  void loopTask(void*)
  {
    setup();
    for (;;) {
      loop();
    }
    vTaskDelete(NULL);
  }

  void app_main()
  {
    xTaskCreatePinnedToCore(loopTask, "loopTask", 8192, NULL, 1, NULL, 1);
  }
}
#endif