// スタックチャンをBluetoothスピーカーとして使うサンプルアプリです。
// https://github.com/m5stack/M5Unified のBluetooth_with_ESP32A2DP.inoをベースにサーボの動きを追加しています。
// Copyright (c) 2022 Takao Akaki

#include <Arduino.h>

#include <Ticker.h>
#include <SD.h>
#include <M5Unified.h>
#include <M5UnitOLED.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>
#include "Stackchan_servo.h"
#include "BluetoothA2DPSink_M5Speaker.hpp"
#include "Avatar.h"

using namespace m5avatar;
Avatar avatar;
StackchanSERVO servo;

M5UnitOLED oled;
M5Canvas canvas(&oled);

static constexpr char text[] = "横浜頂戦!";
static constexpr size_t textlen = sizeof(text) / sizeof(text[0]);
int textpos = 0;
int scrollstep = 2;

#include "Stackchan_system_config.h"

// --------------------
// サーボ関連の初期設定
#define START_DEGREE_VALUE_X 90         // Xサーボの初期位置（変更しないでください。） Start angle of ServoX
#define START_DEGREE_VALUE_Y 90         // Yサーボの初期位置（変更しないでください。） Start angle of ServoY

fs::FS json_fs = SD; // JSONファイルの収納場所(SPIFFS or SD)
StackchanSystemConfig system_config;
const char* stackchan_system_config_yaml = "/yaml/SC_Config.yaml";

bool bluetooth_mode = false; 

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

void oledLoop(void *args) {
  for(;;) {
    int32_t cursor_x = canvas.getCursorX() - scrollstep;
    if (cursor_x <= 0)
    {
      textpos = 0;
      cursor_x = oled.width();
    }

    canvas.setCursor(cursor_x, 0);
    canvas.scroll(-scrollstep, 0);
    while (textpos < textlen && cursor_x <= oled.width())
    {
      canvas.print(text[textpos++]);
      cursor_x = canvas.getCursorX();
    }
    oled.waitDisplay();
    canvas.pushSprite(&oled, 0, (oled.height() - canvas.height()) >> 1);
    vTaskDelay(50/portTICK_PERIOD_MS);
  }
}


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
      int lyric_no = random(system_config.getLyrics_num());
      int exp2 = random(2);
      avatar.setMouthOpenRatio(1.0f);
      avatar.setSpeechText((const char*)system_config.getLyric(lyric_no)->c_str());
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
  if (M5.getBoard() == m5::board_t::board_M5Stack) {
    M5.In_I2C.release();
  }

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

  // BASICとFIREのV2.6で25MHzだと読み込めないため10MHzまで下げています。
  SD.begin(GPIO_NUM_4, SPI, 15000000);
  
  delay(1000);
  system_config.loadConfig(json_fs, stackchan_system_config_yaml);
  
  M5.Speaker.setVolume(system_config.getBluetoothSetting()->start_volume);
  M5.Speaker.setChannelVolume(system_config.getBluetoothSetting()->start_volume, m5spk_virtual_channel);

  bluetooth_mode = system_config.getBluetoothSetting()->starting_state;
  Serial.printf("Bluetooth_mode:%s\n", bluetooth_mode ? "true" : "false");
  
  servo.begin(system_config.getServoInfo()->servo_pin_x, START_DEGREE_VALUE_X,
              system_config.getServoInfo()->servo_offset_x,
              system_config.getServoInfo()->servo_pin_y, START_DEGREE_VALUE_Y,
              system_config.getServoInfo()->servo_offset_y);
  delay(2000);

  avatar.init(); // start drawing

  avatar.addTask(lipSync, "lipSync");
  avatar.addTask(servoLoop, "servoLoop");
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
  oled.init();
  oled.setRotation(3);
  canvas.setColorDepth(1); // mono color
  canvas.setFont(&fonts::lgfxJapanMinchoP_32);
  canvas.setTextWrap(false);
  canvas.setTextSize(2);
  canvas.createSprite(oled.width() + 64, 72);

  xTaskCreateUniversal(oledLoop
                      , "gfxLoop"
                      , 4096
                      , NULL 
                      , 8
                      , NULL
                      , PRO_CPU_NUM);


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
  if (M5.getBoard() == m5::board_t::board_M5StackCore2) {
    if (M5.Power.Axp192.getACINVolatge() < 3.0f) {
      // USBからの給電が停止したとき
      // Serial.println("USBPowerUnPlugged.");
      M5.Power.setLed(0);
      if ((system_config.getAutoPowerOffTime() > 0) and (last_discharge_time == 0)) {
        last_discharge_time = millis();
      } else if ((system_config.getAutoPowerOffTime() > 0) 
                and ((millis() - last_discharge_time) > system_config.getAutoPowerOffTime())) {
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