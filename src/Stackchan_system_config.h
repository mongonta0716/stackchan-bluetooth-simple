#ifndef __STACKCHAN_SYSTEM_CONFIG_H__
#define __STACKCHAN_SYSTEM_CONFIG_H__

//#include <ArduinoJson.h>
#include <M5Unified.h>
#include <ArduinoYaml.h>

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
        uint8_t servo_pin_x;
        uint8_t servo_pin_y;
        uint8_t servo_offset_x;
        uint8_t servo_offset_y;
} servo_initial_param_s;

enum servo_axis {
    AXIS_X,
    AXIS_Y
};

enum AvatarMode {
    NORMAL,
    SINGING
};

class StackchanSystemConfig {
    protected:
        servo_initial_param_s _servo;
        servo_interval_s _servo_interval[2];
        uint8_t _mode_num;
        bluetooth_s _bluetooth;
        uint32_t _auto_power_off_time;
        String _font_language_code;
        String _lyrics[10];
        uint8_t _lyrics_num;

        void setDefaultParameters();
        void setSystemConfig(DynamicJsonDocument doc);

    public:
        StackchanSystemConfig();
        ~StackchanSystemConfig();
        void loadConfig(fs::FS& fs, const char *yaml_filename);

        void printAllParameters();

        servo_initial_param_s* getServoInfo() { return &_servo; }
        servo_interval_s* getServoInterval(AvatarMode avatar_mode) { return &_servo_interval[avatar_mode]; }
        bluetooth_s* getBluetoothSetting() { return &_bluetooth; }
        String* getLyric(uint8_t no) { return &_lyrics[no]; }
        uint8_t getLyrics_num() { return _lyrics_num; }
        uint32_t getAutoPowerOffTime() { return _auto_power_off_time; }
        const lgfx::IFont* getFont();
};

#endif // __STACKCHAN_SYSTEM_CONFIG_H__
