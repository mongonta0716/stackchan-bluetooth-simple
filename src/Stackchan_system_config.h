#ifndef __STACKCHAN_SYSTEM_CONFIG_H__
#define __STACKCHAN_SYSTEM_CONFIG_H__

//#include <ArduinoJson.h>
#include <M5Unified.h>
#include <ArduinoYaml.h>

#include "Stackchan_servo.h"

typedef struct ServoInterval {
    // 下記のminとmaxの間でランダムの値を取ります。
    const char *mode_name;
    uint32_t interval_min; // サーボが停止する間隔（最小）
    uint32_t interval_max; // サーボが停止する間隔（最大）
    uint32_t move_min;     // サーボが移動する時間（最小）
    uint32_t move_max;     // サーボが移動する時間（最大）
} servo_interval_s;

typedef struct Bluetooth {
    String device_name;
    bool starting_state;
    uint8_t start_volume;
} bluetooth_s;

typedef struct ServoInitialParam {
        uint8_t pin;
        uint8_t offset;
        uint8_t upper_limit;
        uint8_t lower_limit;
} servo_initial_param_s;

enum AvatarMode {
    NORMAL,
    SINGING
};

class StackchanSystemConfig {
    protected:
        servo_initial_param_s _servo[2];
        servo_interval_s _servo_interval[2];
        uint8_t _mode_num;
        bluetooth_s _bluetooth;
        uint32_t _auto_power_off_time;                       // USB給電が停止後、電源OFF
        String _font_language_code;                          // フォントコード()
        String _lyrics[10];                                  // 吹き出しに表示するセリフ
        uint8_t _lyrics_num;                                 // 吹き出しに表示するセリフの数
        uint8_t _led_lr;                                     // LEDを光らせる音源を指定（0:stereo, 1:left_only, 2:right_only)

        void setDefaultParameters();
        void setSystemConfig(DynamicJsonDocument doc);

    public:
        StackchanSystemConfig();
        ~StackchanSystemConfig();
        void loadConfig(fs::FS& fs, const char *yaml_filename);

        void printAllParameters();

        servo_initial_param_s* getServoInfo(uint8_t servo_axis_no) { return &_servo[servo_axis_no]; }
        servo_interval_s* getServoInterval(AvatarMode avatar_mode) { return &_servo_interval[avatar_mode]; }
        bluetooth_s* getBluetoothSetting() { return &_bluetooth; }
        String* getLyric(uint8_t no) { return &_lyrics[no]; }
        uint8_t getLyrics_num() { return _lyrics_num; }
        uint32_t getAutoPowerOffTime() { return _auto_power_off_time; }
        const lgfx::IFont* getFont();
        uint8_t getLedLR() { return _led_lr; }
};

#endif // __STACKCHAN_SYSTEM_CONFIG_H__
