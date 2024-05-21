// スタックチャンをBluetoothスピーカーとして使うサンプルアプリです。
// https://github.com/m5stack/M5Unified のBluetooth_with_ESP32A2DP.inoをベースにサーボの動きを追加しています。
// Copyright (c) 2022 Takao Akaki

#include <Arduino.h>

#include <Ticker.h>
#include <SD.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>
#include "Stackchan_servo.h"
#include "BluetoothA2DPSink_M5Speaker.hpp"
#include "Avatar.h"
#include "Stackchan_Takao_Base.hpp"

using namespace m5avatar;
Avatar avatar;
StackchanSERVO servo;

#include "Stackchan_system_config.h"

// M5GoBottomのLEDを使わない場合は下記の1行をコメントアウトしてください。
#define USE_LED

#ifdef USE_LED
  #include <FastLED.h>
  #define NUM_LEDS 10
#if defined(ARDUINO_M5STACK_FIRE) || defined(ARDUINO_M5Stack_Core_ESP32)
  // M5Core1 + M5GoBottom1の組み合わせ
  #define LED_PIN 15
#else
  // M5Core2 + M5GoBottom2の組み合わせ
  #define LED_PIN 25
#endif
  CRGB leds[NUM_LEDS];
  #ifdef USE_LED_OUT
  CRGB leds_out[NUM_LED_OUT];
  #endif

  CHSV red (0, 255, 255);
  CHSV green (95, 255, 255);
  CHSV blue (160, 255, 255);
  CHSV magenta (210, 255, 255);
  CHSV yellow (45, 255, 255);
  CHSV hsv_table[5] = { blue, green, yellow, magenta, red };
  CHSV hsv_table_out[5] = { blue, green, yellow, magenta, red }; //red, magenta, yellow, green, blue };

  void turn_off_led() {
    // Now turn the LED off, then pause
    for(int i=0;i<NUM_LEDS;i++) leds[i] = CRGB::Black;
    FastLED.show();  
  }

  void clear_led_buff() {
    // Now turn the LED off, then pause
    for(int i=0;i<NUM_LEDS;i++) leds[i] =  CRGB::Black;
  }

  void level_led(int level1, int level2) {  
  if(level1 > 5) level1 = 5;
  if(level2 > 5) level2 = 5;
    
    clear_led_buff(); 
    for(int i=0;i<level1;i++){
      fill_gradient(leds, 0, hsv_table[i], 4, hsv_table[0] );
      #ifdef USE_LED_OUT
      fill_gradient(leds_out, 0, hsv_table_out[0], 18, hsv_table_out[i] );
      #endif
    }
    for(int i=0;i<level2;i++){
      fill_gradient(leds, 5, hsv_table[0], 9, hsv_table[i] );
      #ifdef USE_LED_OUT
      fill_gradient(leds_out, 19, hsv_table_out[i], 36, hsv_table_out[0] );
      #endif
    }
    FastLED.show();
  }
#endif



fs::FS json_fs = SD; // JSONファイルの収納場所(SPIFFS or SD)
StackchanSystemConfig system_config;
const char* stackchan_system_config_yaml = "/yaml/SC_Config.yaml";

const unsigned long powericon_interval = 3000;  // バッテリーアイコンを更新する間隔(msec)
unsigned long last_powericon_millis = 0;

bool bluetooth_mode = false; 

// --------------------
// Avatar関連の初期設定
#define LIPSYNC_LEVEL_MAX 10.0f
static float lipsync_level_max = LIPSYNC_LEVEL_MAX; // リップシンクの上限初期値
float mouth_ratio = 0.0f;
bool sing_happy = true;
ColorPalette *cps;
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
  DriveContext *ctx = (DriveContext *)args;
  Avatar *avatar = ctx->getAvatar();
  long move_time = 0;
  long interval_time = 0;
  long move_x = 0;
  long move_y = 0;
  float gaze_x = 0.0f;
  float gaze_y = 0.0f;
  bool sing_mode = false;
  while (avatar->isDrawing()) {
    if (mouth_ratio == 0.0f) {
      // 待機時の動き
      interval_time = random(system_config.getServoInterval(AvatarMode::NORMAL)->interval_min
                           , system_config.getServoInterval(AvatarMode::NORMAL)->interval_max);
      move_time = random(system_config.getServoInterval(AvatarMode::NORMAL)->move_min
                       , system_config.getServoInterval(AvatarMode::NORMAL)->move_max);
      lipsync_level_max = LIPSYNC_LEVEL_MAX; // リップシンク上限の初期化
      sing_mode = false;

    } else {
      // 歌うモードの動き
      interval_time = random(system_config.getServoInterval(AvatarMode::SINGING)->interval_min
                           , system_config.getServoInterval(AvatarMode::SINGING)->interval_max);
      move_time = random(system_config.getServoInterval(AvatarMode::SINGING)->move_min
                       , system_config.getServoInterval(AvatarMode::SINGING)->move_max);
      sing_mode = true;
    } 
    avatar->getGaze(&gaze_y, &gaze_x);
    
//    Serial.printf("x:%f:y:%f\n", gaze_x, gaze_y);
    // X軸は90°から+-で左右にスイング
    if (gaze_x < 0) {
      move_x = system_config.getServoInfo(AXIS_X)->start_degree - mouth_ratio * 15 + (int)(30.0 * gaze_x);
    } else {
      move_x = system_config.getServoInfo(AXIS_X)->start_degree + mouth_ratio * 15 + (int)(30.0 * gaze_x);
    }
    // Y軸は90°から上にスイング（最大35°）
    move_y = system_config.getServoInfo(AXIS_Y)->start_degree - mouth_ratio * 10 - abs(25.0 * gaze_y);
    servo.moveXY(move_x, move_y, move_time);
    if (!bluetooth_mode) {
      int lyric_no = random(system_config.getLyrics_num());
      int exp2 = random(2);
      avatar->setMouthOpenRatio(1.0f);
      avatar->setSpeechText((const char*)system_config.getLyric(lyric_no)->c_str());
      if (exp2 % 2) {
        avatar->setExpression(Expression::Neutral);
      } else {
        avatar->setExpression(Expression::Happy);
      }
      avatar->setMouthOpenRatio(0.5f);
    }
    //avatar.setExpression((Expression)exp);
    if (sing_mode) {
      // 歌っているときはうなずく
      servo.moveXY(move_x, move_y + 10, 400);
    }
    vTaskDelay(interval_time/portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}


void lipSync(void *args)
{
  DriveContext *ctx = (DriveContext *)args;
  Avatar *avatar = ctx->getAvatar();
  while (avatar->isDrawing())
  {
    uint64_t level = 0;
    auto buf = a2dp_sink.getBuffer();
    if (buf) {
#ifdef USE_LED
      // buf[0]: LEFT
      // buf[1]: RIGHT
      switch(system_config.getLedLR()) {
        case 1: // Left Only
          level_led(abs(buf[0])*10/INT16_MAX,abs(buf[0])*10/INT16_MAX);
          break;
        case 2: // Right Only
          level_led(abs(buf[1])*10/INT16_MAX,abs(buf[1])*10/INT16_MAX);
          break;
        default: // Stereo
          level_led(abs(buf[1])*10/INT16_MAX,abs(buf[0])*10/INT16_MAX);
          break;
      }
#endif

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
  vTaskDelete(NULL);
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

void avatarStart() {
  avatar.start();  
  avatar.addTask(lipSync, "lipSync");
  avatar.addTask(servoLoop, "servoLoop");
}
void avatarStop() {
  avatar.stop();  
}



void setup(void)
{
  auto cfg = M5.config();
#ifndef ARDUINO_M5STACK_Core2
  cfg.output_power = true;
#endif
//cfg.external_spk = true;    /// use external speaker (SPK HAT / ATOMIC SPK)
//cfg.external_spk_detail.omit_atomic_spk = true; // exclude ATOMIC SPK
//cfg.external_spk_detail.omit_spk_hat    = true; // exclude SPK HAT

  M5.begin(cfg);

  { /// custom setting
    auto spk_cfg = M5.Speaker.config();
    /// Increasing the sample_rate will improve the sound quality instead of increasing the CPU load.
#ifndef ARDUINO_M5Stack_Core_ESP32
    // M5Stack Fire/Core2/AWS 向けPSRAM搭載機種のパラメータ
    spk_cfg.sample_rate = 96000; // default:64000 (64kHz)  e.g. 48000 , 50000 , 80000 , 96000 , 100000 , 128000 , 144000 , 192000 , 200000
    spk_cfg.task_pinned_core = APP_CPU_NUM;
    // spk_cfg.task_priority = configMAX_PRIORITIES - 2;
    spk_cfg.dma_buf_count = 20;
    //spk_cfg.stereo = true;
    spk_cfg.dma_buf_len = 256;
#else
    // M5Stack Basic/Gray/Goのパラメータ
    // 音が途切れる場合はdma_buf_countとdma_buf_lenを増やすと改善できる場合あり。
    // 顔が表示されなかったり、点滅するようであれば増やし過ぎなので減らしてください。
    spk_cfg.sample_rate = 64000; // default:64000 (64kHz)  e.g. 48000 , 50000 , 80000 , 96000 , 100000 , 128000 , 144000 , 192000 , 200000
    spk_cfg.task_pinned_core = APP_CPU_NUM;
    // spk_cfg.task_priority = configMAX_PRIORITIES - 2;
    spk_cfg.dma_buf_count = 10;
    //spk_cfg.stereo = true;
    spk_cfg.dma_buf_len = 192;
#endif
    M5.Speaker.config(spk_cfg);
  }


  M5.Speaker.begin();

  // BASICとFIREのV2.6で25MHzだと読み込めないため15MHzまで下げています。
  SD.begin(GPIO_NUM_4, SPI, 15000000);
  
  delay(1000);
  system_config.loadConfig(json_fs, stackchan_system_config_yaml);

  M5.Speaker.setVolume(system_config.getBluetoothSetting()->start_volume);
  M5.Speaker.setChannelVolume(system_config.getBluetoothSetting()->start_volume, m5spk_virtual_channel);

  if (system_config.getUseTakaoBase()) {
    M5.Power.setExtOutput(false);  // TakaoBaseの後ろ給電を使用する。
  } else {
    M5.Power.setExtOutput(true);  // M5Stackの横から給電を使用する。
  }

  bluetooth_mode = system_config.getBluetoothSetting()->starting_state;
  Serial.printf("Bluetooth_mode:%s\n", bluetooth_mode ? "true" : "false");

  if ((system_config.getServoInfo(AXIS_X)->pin == 21)
     || (system_config.getServoInfo(AXIS_X)->pin == 22)) {
    // Port.Aを利用する場合は、I2Cが使えないのでアイコンは表示しない。
    avatar.setBatteryIcon(false);
    if (M5.getBoard() == m5::board_t::board_M5Stack) {
      M5.In_I2C.release();
    }
  } else {
    avatar.setBatteryIcon(true);
    avatar.setBatteryStatus(M5.Power.isCharging(), M5.Power.getBatteryLevel());
  }
  
  servo.begin(system_config.getServoInfo(AXIS_X)->pin, system_config.getServoInfo(AXIS_X)->start_degree,
              system_config.getServoInfo(AXIS_X)->offset,
              system_config.getServoInfo(AXIS_Y)->pin, system_config.getServoInfo(AXIS_Y)->start_degree,
              system_config.getServoInfo(AXIS_Y)->offset,
              (ServoType)system_config.getServoType());
  delay(2000);

  avatar.init(1); // start drawing
  cps = new ColorPalette();

  // Avatarの色を変えたい場合は下記の2行のカラーコードを書き換えてください。
  cps->set(COLOR_PRIMARY, TFT_WHITE);         // 16進数で指定する場合は下記のように記述
  //cps->set(COLOR_PRIMARY, (uint16_t)0x000000);         // 16進数で指定する場合は下記のように記述
  cps->set(COLOR_BACKGROUND, TFT_BLACK);      // (uint16_t)0xaabbcc
  //cps->set(COLOR_BACKGROUND, (uint16_t)0xdab300);
  avatar.setColorPalette(*cps);
  last_powericon_millis = millis();

  avatar.addTask(lipSync, "lipSync");
  avatar.addTask(servoLoop, "servoLoop", 2048U, 1);
  avatar.setExpression(Expression::Neutral);
  avatar.setSpeechFont(system_config.getFont());

  if (bluetooth_mode) {
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.setHvtEventCallback(hvt_event_callback);
    a2dp_sink.start(system_config.getBluetoothSetting()->device_name.c_str(), true);
    avatar.setExpression(Expression::Sad);
    avatar.setSpeechText("Bluetooth Mode");
  } else {
    avatar.setSpeechText("Normal Mode");
  }

#ifdef USE_LED
  FastLED.addLeds<SK6812, LED_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
  FastLED.setBrightness(32);
  level_led(5, 5);
  delay(1000);
  turn_off_led();
#endif


}

void loop(void)
{

  M5.update();
  if (M5.BtnA.wasDecideClickCount())
  {
    switch (M5.BtnA.getClickCount())
    {
    case 1:
      if (!bluetooth_mode) {
        a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
        a2dp_sink.setHvtEventCallback(hvt_event_callback);
        a2dp_sink.start(system_config.getBluetoothSetting()->device_name.c_str(), true);
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
    M5.Speaker.setVolume(volume);
    M5.Speaker.setChannelVolume(m5spk_virtual_channel, volume);
    M5.Speaker.tone(2000, 100);
    delay(200);
    M5.Speaker.tone(1000, 100);
  }
  if (M5.BtnC.wasPressed()) {
    uint8_t volume = M5.Speaker.getChannelVolume(m5spk_virtual_channel);
    volume = volume + 10;
    M5.Speaker.setVolume(volume);
    M5.Speaker.setChannelVolume(m5spk_virtual_channel, volume);
    M5.Speaker.tone(1000, 100);
    delay(200);
    M5.Speaker.tone(2000, 100);
  }
  if ((millis() - last_powericon_millis)> powericon_interval) {
/*
  Serial.printf("esp_get_free_heap_size()                              : %6d\n", esp_get_free_heap_size() );
  Serial.printf("esp_get_minimum_free_heap_size()                      : %6d\n", esp_get_minimum_free_heap_size() );
  //xPortGetFreeHeapSize()（データメモリ）ヒープの空きバイト数を返すFreeRTOS関数です。これはを呼び出すのと同じheap_caps_get_free_size(MALLOC_CAP_8BIT)です。
  Serial.printf("xPortGetFreeHeapSize()                                : %6d\n", xPortGetFreeHeapSize() );
  //xPortGetMinimumEverFreeHeapSize()また、関連heap_caps_get_minimum_free_size()するものを使用して、ブート以降のヒープの「最低水準点」を追跡できます。
  Serial.printf("xPortGetMinimumEverFreeHeapSize()                     : %6d\n", xPortGetMinimumEverFreeHeapSize() );
  //heap_caps_get_free_size() さまざまなメモリ機能の現在の空きメモリを返すためにも使用できます。
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_EXEC)              : %6d\n", heap_caps_get_free_size(MALLOC_CAP_EXEC) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_32BIT)             : %6d\n", heap_caps_get_free_size(MALLOC_CAP_32BIT) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_8BIT)              : %6d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_DMA)               : %6d\n", heap_caps_get_free_size(MALLOC_CAP_DMA) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_PID2)              : %6d\n", heap_caps_get_free_size(MALLOC_CAP_PID2) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_PID3)              : %6d\n", heap_caps_get_free_size(MALLOC_CAP_PID3) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_PID3)              : %6d\n", heap_caps_get_free_size(MALLOC_CAP_PID4) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_PID4)              : %6d\n", heap_caps_get_free_size(MALLOC_CAP_PID5) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_PID5)              : %6d\n", heap_caps_get_free_size(MALLOC_CAP_PID6) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_PID6)              : %6d\n", heap_caps_get_free_size(MALLOC_CAP_PID7) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_PID7)              : %6d\n", heap_caps_get_free_size(MALLOC_CAP_PID3) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_SPIRAM)            : %6d\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_INTERNAL)          : %6d\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_DEFAULT)           : %6d\n", heap_caps_get_free_size(MALLOC_CAP_DEFAULT) );
  //Serial.printf("heap_caps_get_free_size(MALLOC_CAP_IRAM_8BIT)         : %6d\n", heap_caps_get_free_size(MALLOC_CAP_IRAM_8BIT) );
  Serial.printf("heap_caps_get_free_size(MALLOC_CAP_INVALID)           : %6d\n", heap_caps_get_free_size(MALLOC_CAP_INVALID) );
*/
  Serial.printf("free_block DMA: %6d\n", heap_caps_get_largest_free_block(MALLOC_CAP_DMA));
#if !defined(ARDUINO_M5Stack_Core_ESP32) && !defined(ARDUINO_M5STACK_FIRE)
  if (M5.getBoard() == m5::board_t::board_M5StackCore2) {
    // Stack-chan_Takao_Baseを使わない場合
    if ((M5.Power.Axp192.getACINVoltage() < 3.0f) && (M5.Power.Ina3221[0].getBusVoltage(2) < 3.0f)) {
      // USBからの給電が停止したとき
      // Serial.println("USBPowerUnPlugged.");
      M5.Power.setLed(0);
      if ((system_config.getAutoPowerOffTime() > 0) and (last_discharge_time == 0)) {
        M5.Speaker.tone(500, 100);
        last_discharge_time = millis();
      } else if ((system_config.getAutoPowerOffTime() > 0) 
                and ((millis() - last_discharge_time) > system_config.getAutoPowerOffTime())) {
        M5.Power.setExtOutput(false);
        M5.Speaker.tone(2000, 100);
        delay(200);
        M5.Speaker.tone(1000, 100);
        delay(200);
        M5.Speaker.tone(500, 100);
        delay(1000);
        M5.Power.powerOff();
      }
    } else {
      //Serial.println("USBPowerPlugged.");
      M5.Power.setLed(80);
      if (last_discharge_time > 0) {
        last_discharge_time = 0;
      }
    }
  }
#endif
    if ((system_config.getServoInfo(AXIS_X)->pin != 21)
      && (system_config.getServoInfo(AXIS_X)->pin != 22)) {
      // Port.Aを利用する場合は、I2Cが使えないのでアイコンは表示しない。
      avatar.setBatteryStatus(M5.Power.isCharging(), M5.Power.getBatteryLevel());
      last_powericon_millis = millis();
    }
  }

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