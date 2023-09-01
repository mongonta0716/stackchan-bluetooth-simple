// Copyright (c) Takao Akaki
#include "Stackchan_servo.h"

#include <ServoEasing.hpp>

long convertSCS0009Pos(int16_t degree) {
  //Serial.printf("Degree: %d\n", degree);
  return map(degree, 0, 300, 1023, 0);
}

StackchanSERVO::StackchanSERVO() {}

StackchanSERVO::~StackchanSERVO() {}


void StackchanSERVO::attachServos() {
  if (_servo_type == SCS) {
    // SCS0009
    Serial2.begin(1000000, SERIAL_8N1, _init_param.servo[AXIS_X].pin, _init_param.servo[AXIS_Y].pin);
    _sc.pSerial = &Serial2;
    _sc.WritePos(AXIS_X + 1, convertSCS0009Pos(_init_param.servo[AXIS_X].start_degree + _init_param.servo[AXIS_X].offset), 1000);
    _sc.WritePos(AXIS_Y + 1, convertSCS0009Pos(_init_param.servo[AXIS_Y].start_degree + _init_param.servo[AXIS_Y].offset), 1000);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  } else {
    // SG90 PWM
    if (_servo_x.attach(_init_param.servo[AXIS_X].pin, 
                        _init_param.servo[AXIS_X].start_degree + _init_param.servo[AXIS_X].offset,
                        DEFAULT_MICROSECONDS_FOR_0_DEGREE,
                        DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
      Serial.print("Error attaching servo x");
    }
    if (_servo_y.attach(_init_param.servo[AXIS_Y].pin, 
                        _init_param.servo[AXIS_Y].start_degree + _init_param.servo[AXIS_Y].offset,
                        DEFAULT_MICROSECONDS_FOR_0_DEGREE,
                        DEFAULT_MICROSECONDS_FOR_180_DEGREE)) {
      Serial.print("Error attaching servo x");
    }

    _servo_x.setEasingType(EASE_QUADRATIC_IN_OUT);
    _servo_y.setEasingType(EASE_QUADRATIC_IN_OUT);
  }
}

void StackchanSERVO::begin(stackchan_servo_initial_param_s init_param) {
  _init_param = init_param;
  attachServos();
}

void StackchanSERVO::begin(int servo_pin_x, int16_t start_degree_x, int16_t offset_x, 
                           int servo_pin_y, int16_t start_degree_y, int16_t offset_y,
                           ServoType servo_type) {
  _init_param.servo[AXIS_X].pin          = servo_pin_x;
  _init_param.servo[AXIS_X].start_degree = start_degree_x;
  _init_param.servo[AXIS_X].offset       = offset_x;
  _init_param.servo[AXIS_Y].pin          = servo_pin_y;
  _init_param.servo[AXIS_Y].start_degree = start_degree_y;
  _init_param.servo[AXIS_Y].offset       = offset_y;
  _servo_type = servo_type;
  attachServos();
}

void StackchanSERVO::moveX(int x, uint32_t millis_for_move) {
  if (_servo_type == SCS) {
    _sc.WritePos(AXIS_X + 1, convertSCS0009Pos(x + _init_param.servo[AXIS_X].offset), millis_for_move);
    _isMoving = true;
    vTaskDelay(millis_for_move/portTICK_PERIOD_MS);
    _isMoving = false;
  } else {
    if (millis_for_move == 0) {
      _servo_x.easeTo(x + _init_param.servo[AXIS_X].offset);
    } else {
      _servo_x.easeToD(x + _init_param.servo[AXIS_X].offset, millis_for_move);
    }
    _isMoving = true;
    synchronizeAllServosStartAndWaitForAllServosToStop();
    _isMoving = false;
  }
}

void StackchanSERVO::moveX(servo_param_s servo_param_x) {
  _init_param.servo[AXIS_X].offset = servo_param_x.offset;
  moveX(servo_param_x.degree, servo_param_x.millis_for_move);
}

void StackchanSERVO::moveY(int y, uint32_t millis_for_move) {
  if (_servo_type == SCS) {
    _sc.WritePos(AXIS_Y + 1, convertSCS0009Pos(y + _init_param.servo[AXIS_Y].offset), millis_for_move);
    _isMoving = true;
    vTaskDelay(millis_for_move/portTICK_PERIOD_MS);
    _isMoving = false;
  } else {
    if (millis_for_move == 0) {
      _servo_y.easeTo(y + _init_param.servo[AXIS_Y].offset);
    } else {
      _servo_y.easeToD(y + _init_param.servo[AXIS_Y].offset, millis_for_move);
    }
    _isMoving = true;
    synchronizeAllServosStartAndWaitForAllServosToStop();
    _isMoving = false;
  }
}

void StackchanSERVO::moveY(servo_param_s servo_param_y) {
  _init_param.servo[AXIS_Y].offset = servo_param_y.offset;
  moveY(servo_param_y.degree, servo_param_y.millis_for_move);
}
void StackchanSERVO::moveXY(int x, int y, uint32_t millis_for_move) {
  if (_servo_type == SCS) {
    _sc.WritePos(AXIS_X + 1, convertSCS0009Pos(x + _init_param.servo[AXIS_X].offset), millis_for_move);
    _sc.WritePos(AXIS_Y + 1, convertSCS0009Pos(y + _init_param.servo[AXIS_Y].offset), millis_for_move);
    _isMoving = true;
    vTaskDelay(millis_for_move/portTICK_PERIOD_MS);
    _isMoving = false;
  } else {
    _servo_x.setEaseToD(x + _init_param.servo[AXIS_X].offset, millis_for_move);
    _servo_y.setEaseToD(y + _init_param.servo[AXIS_Y].offset, millis_for_move);
    _isMoving = true;
    synchronizeAllServosStartAndWaitForAllServosToStop();
    _isMoving = false;
  }
}

void StackchanSERVO::moveXY(servo_param_s servo_param_x, servo_param_s servo_param_y) {
  if (_servo_type == SCS) {
    _sc.WritePos(AXIS_X + 1, convertSCS0009Pos(servo_param_x.degree + servo_param_x.offset), servo_param_x.millis_for_move);
    _sc.WritePos(AXIS_Y + 1, convertSCS0009Pos(servo_param_y.degree + servo_param_y.offset), servo_param_y.millis_for_move);
    _isMoving = true;
    vTaskDelay(max(servo_param_x.millis_for_move, servo_param_y.millis_for_move)/portTICK_PERIOD_MS);
    _isMoving = false;
  } else {
    if (servo_param_x.degree != 0) {
      _servo_x.setEaseToD(servo_param_x.degree + servo_param_x.offset, servo_param_x.millis_for_move);
    }
    if (servo_param_y.degree != 0) {
      _servo_y.setEaseToD(servo_param_y.degree + servo_param_y.offset, servo_param_y.millis_for_move);
    }
    _isMoving = true;
    synchronizeAllServosStartAndWaitForAllServosToStop();
    _isMoving = false;
  }
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
    moveXY(_init_param.servo[AXIS_X].start_degree, _init_param.servo[AXIS_Y].degree, 1000);
}
