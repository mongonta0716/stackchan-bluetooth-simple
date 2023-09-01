#include "Stackchan_system_config.h"


StackchanSystemConfig::StackchanSystemConfig() {

};

StackchanSystemConfig::~StackchanSystemConfig() {

}

void StackchanSystemConfig::setDefaultParameters() {
    switch(M5.getBoard()) {
        case m5::board_t::board_M5StackCore2:
            _servo[AXIS_X].pin = 33;
            _servo[AXIS_Y].pin = 32;
            break;
        case m5::board_t::board_M5Stack:
            _servo[AXIS_X].pin = 22;
            _servo[AXIS_Y].pin = 21;
            break;
        default:
            Serial.printf("UnknownBoard:%d\n", M5.getBoard());
            _servo[AXIS_X].pin = 22;
            _servo[AXIS_Y].pin = 21;
            break;
    }
    _servo[AXIS_X].offset = 0;
    _servo[AXIS_X].lower_limit = 0;
    _servo[AXIS_X].upper_limit = 180;
    _servo[AXIS_X].start_degree = 90;
    _servo[AXIS_Y].offset = 0;
    _servo[AXIS_Y].lower_limit = 50;
    _servo[AXIS_Y].upper_limit = 90;
    _servo[AXIS_Y].start_degree = 90;
    _servo_interval[0].mode_name = "normal";
    _servo_interval[0].interval_min = 5000;
    _servo_interval[0].interval_max = 10000;
    _servo_interval[0].move_min = 500;
    _servo_interval[0].move_max = 1500;
    _servo_interval[1].mode_name = "sing_mode";
    _servo_interval[1].interval_min = 1000;
    _servo_interval[1].interval_max = 2000;
    _servo_interval[1].move_min = 500;
    _servo_interval[1].move_max = 1500;
    _mode_num = 2;
    _bluetooth.device_name = "M5Stack_BTSPK";
    _bluetooth.starting_state = true;
    _bluetooth.start_volume = 150;
    _auto_power_off_time = 0;
    _font_language_code = "JA";
    _lyrics[0] = "こんにちは";
    _lyrics[1] = "Hello";
    _lyrics[2] = "你好";
    _lyrics[3] = "Bonjour";
    _lyrics_num = 4;
    _led_lr = 0;
    _led_pin = -1;
    _takao_base = false;
    _servo_type = 0;
    _servo[AXIS_X].start_degree = 90;
    _servo[AXIS_Y].start_degree = 90;
}

void StackchanSystemConfig::loadConfig(fs::FS& fs, const char *yaml_filename) {
    Serial.printf("----- StackchanSystemConfig::loadConfig:%s\n", yaml_filename);
    File file = fs.open(yaml_filename);
    if (file) {
        DynamicJsonDocument doc(2048);
        auto err = deserializeYml( doc, file);
        if (err) {
            Serial.printf("yaml file read error: %s\n", yaml_filename);
            Serial.printf("error%s\n", err.c_str());
        }
        serializeJsonPretty(doc, Serial);
        setSystemConfig(doc);
    } else {
        Serial.println("ConfigFile Not Found. Default Parameters used.");
        // JSONファイルが見つからない場合はデフォルト値を利用します。
        setDefaultParameters();
    }
    printAllParameters();
}

void StackchanSystemConfig::setSystemConfig(DynamicJsonDocument doc) {
    JsonObject servo = doc["servo"];
    _servo[AXIS_X].pin = servo["pin"]["x"];
    _servo[AXIS_Y].pin = servo["pin"]["y"];

    _servo[AXIS_X].offset = servo["offset"]["x"];
    _servo[AXIS_Y].offset = servo["offset"]["y"];

    _servo[AXIS_X].lower_limit = servo["lower_limit"]["x"];
    _servo[AXIS_X].upper_limit = servo["upper_limit"]["x"];
    _servo[AXIS_Y].lower_limit = servo["lower_limit"]["y"];
    _servo[AXIS_Y].upper_limit = servo["upper_limit"]["y"];
    int i = 0;
    for (JsonPair servo_speed_item : servo["speed"].as<JsonObject>()) {
        _servo_interval[i].mode_name = servo_speed_item.key().c_str();
        _servo_interval[i].interval_min = servo_speed_item.value()["interval_min"];
        _servo_interval[i].interval_max = servo_speed_item.value()["interval_max"];
        _servo_interval[i].move_min = servo_speed_item.value()["move_min"];
        _servo_interval[i].move_max = servo_speed_item.value()["move_max"];
        i++;
    }
    _mode_num = i;

    _bluetooth.device_name = doc["bluetooth"]["device_name"].as<String>();
    _bluetooth.starting_state = doc["bluetooth"]["starting_state"];//.as<bool>();
    _bluetooth.start_volume = doc["bluetooth"]["start_volume"];

    _auto_power_off_time = doc["auto_power_off_time"];
    _font_language_code = doc["balloon"]["font_language"].as<String>();
    
    JsonArray balloon_lyrics = doc["balloon"]["lyrics"];
        
    _lyrics_num = balloon_lyrics.size();
    Serial.printf("lyrics_num:%d\n", _lyrics_num);
    for (int j=0;j<_lyrics_num;j++) {
        _lyrics[j] = balloon_lyrics[j].as<String>();
    }
    _led_lr = doc["led_lr"];
    _led_pin = doc["led_pin"];
    _takao_base = doc["takao_base"];
    _servo_type_str = doc["servo_type"].as<String>();
    if (_servo_type_str.indexOf("SCS") != -1) {
        // SCS0009
        _servo_type = 1;
        _servo[AXIS_X].start_degree = 150;
        _servo[AXIS_Y].start_degree = 150;
    } else {
        _servo_type = 0; // PWMサーボ
        _servo[AXIS_X].start_degree = 90;
        _servo[AXIS_Y].start_degree = 90;
    }

}

const lgfx::IFont* StackchanSystemConfig::getFont() {
    if (_font_language_code.compareTo("JA")) {
        return &fonts::efontJA_16;
    } else if (_font_language_code.compareTo("CN")) {
        return &fonts::efontCN_16;
    } else {
        Serial.printf("FontCodeError:%s\n", _font_language_code);
        return &fonts::Font0;
    }
} 

void StackchanSystemConfig::printAllParameters() {
    Serial.printf("servo:pin_x:%d\n", _servo[AXIS_X].pin);
    Serial.printf("servo:pin_y:%d\n", _servo[AXIS_Y].pin);
    Serial.printf("servo:offset_x:%d\n", _servo[AXIS_X].offset);
    Serial.printf("servo:offset_y:%d\n", _servo[AXIS_Y].offset);
    for (int i=0;i<_mode_num;i++) {
        Serial.printf("mode:%s\n", _servo_interval[i].mode_name);
        Serial.printf("interval_min:%d\n", _servo_interval[i].interval_min);
        Serial.printf("interval_max:%d\n", _servo_interval[i].interval_max);
        Serial.printf("move_min:%d\n", _servo_interval[i].move_min);
        Serial.printf("move_max:%d\n", _servo_interval[i].move_max);
    }
    Serial.printf("mode_num:%d\n", _mode_num);
    Serial.printf("Bluetooth_device_name:%s\n", _bluetooth.device_name.c_str());
    Serial.printf("Bluetooth_starting_state:%s\n", _bluetooth.starting_state ? "true":"false");
    Serial.printf("Bluetooth_start_volume:%d\n", _bluetooth.start_volume);
    Serial.printf("auto_power_off_time:%d\n", _auto_power_off_time);
    Serial.printf("font_language:%s\n", _font_language_code);
    for (int i=0;i<_lyrics_num;i++) {
        Serial.printf("lyrics:%d:%s\n", i, _lyrics[i].c_str());
    }
    Serial.printf("led_lr:%d\n", _led_lr);
    Serial.printf("led_pin:%d\n", _led_pin);
    Serial.printf("use takao_base:%s\n", _takao_base ? "true":"false");
    Serial.printf("ServoTypeStr:%s\n", _servo_type_str.c_str());
    Serial.printf("ServoType: %d\n", _servo_type);
}