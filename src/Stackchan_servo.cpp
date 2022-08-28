// Copyright (c) Takao Akaki
#include <Stackchan_servo.h>

#include <ServoEasing.hpp>
StackchanSERVO::StackchanSERVO() {}

StackchanSERVO::~StackchanSERVO() {}


void StackchanSERVO::attachServos() {
  if (_servo_x.attach(_init_param.x.servo_pin, 
                      _init_param.x.start_degree + _init_param.x.offset,
                      DEFAULT_MICROSECONDS_FOR_0_DEGREE,
                      DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
    Serial.print("Error attaching servo x");
  }
  if (_servo_y.attach(_init_param.y.servo_pin, 
                      _init_param.y.start_degree + _init_param.y.offset,
                      DEFAULT_MICROSECONDS_FOR_0_DEGREE,
                      DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
    Serial.print("Error attaching servo x");
  }

  _servo_x.setEasingType(EASE_QUADRATIC_IN_OUT);
  _servo_y.setEasingType(EASE_QUADRATIC_IN_OUT);
}

void StackchanSERVO::begin(stackchan_servo_initial_param_s init_param) {
  _init_param = init_param;
  attachServos();
}

void StackchanSERVO::begin(int servo_pin_x, int8_t start_degree_x, int8_t offset_x, 
                           int servo_pin_y, int8_t start_degree_y, int8_t offset_y) {
  _init_param.x.servo_pin    = servo_pin_x;
  _init_param.x.start_degree = start_degree_x;
  _init_param.x.offset       = offset_x;
  _init_param.y.servo_pin    = servo_pin_y;
  _init_param.y.start_degree = start_degree_y;
  _init_param.y.offset       = offset_y;
  attachServos();
}

void StackchanSERVO::moveX(int x, uint32_t millis_for_move) {
  if (millis_for_move == 0) {
    _servo_x.easeTo(x + _init_param.x.offset);
  } else {
    _servo_x.easeToD(x + _init_param.x.offset, millis_for_move);
  }
}

void StackchanSERVO::moveX(servo_param_s servo_param_x) {
  _init_param.x.offset = servo_param_x.offset;
  moveX(servo_param_x.degree, servo_param_x.millis_for_move);
}

void StackchanSERVO::moveY(int y, uint32_t millis_for_move) {
  if (millis_for_move == 0) {
    _servo_y.easeTo(y + _init_param.y.offset);
  } else {
    _servo_y.easeToD(y + _init_param.y.offset, millis_for_move);
  }
}

void StackchanSERVO::moveY(servo_param_s servo_param_y) {
  _init_param.y.offset = servo_param_y.offset;
  moveX(servo_param_y.degree, servo_param_y.millis_for_move);
}
void StackchanSERVO::moveXY(int x, int y, uint32_t millis_for_move) {
  _servo_x.setEaseToD(x + _init_param.x.offset, millis_for_move);
  _servo_y.setEaseToD(y + _init_param.y.offset, millis_for_move);
  synchronizeAllServosStartAndWaitForAllServosToStop();
}

void StackchanSERVO::moveXY(servo_param_s servo_param_x, servo_param_s servo_param_y) {
  if (servo_param_x.degree != 0) {
    _servo_x.setEaseToD(servo_param_x.degree + servo_param_x.offset, servo_param_x.millis_for_move);
  }
  if (servo_param_y.degree != 0) {
    _servo_y.setEaseToD(servo_param_y.degree + servo_param_y.offset, servo_param_y.millis_for_move);
  }
  synchronizeAllServosStartAndWaitForAllServosToStop();
}

void StackchanSERVO::motion(Motion motion_number) {
    if (motion_number == nomove) return; 
    moveXY(90, 75, 500);
    switch(motion_number) {
        case greet: 
            moveY(90, 1000);
            moveY(75, 1000);
            break;
        case laugh:
            for (int i=0; i<5; i++) {
                moveY(80, 500);
                moveY(60, 500);
            }
            break;
        case nod:
            for (int i=0; i<5; i++) {
                moveY(90, 1000);
                moveY(60, 1000);
            }
            break;
        case refuse:
            for (int i=0; i<2; i++) {
                moveX(70,  500);
                moveX(110, 500);
            }
            break;
        case test:
            moveX(45,  1000);
            moveX(135, 1000);
            moveX(90, 1000);
            moveY(50, 1000);
            moveY(90, 1000);
            break;
        default:
            Serial.printf("invalid motion number: %d\n", motion_number);
            break;
    }
    delay(1000);
    moveXY(_init_param.x.start_degree, _init_param.y.start_degree, 1000);
}
