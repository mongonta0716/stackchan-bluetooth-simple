#ifndef STACKCHAN_TAKAO_BASE_HPP
#define STACKCHAN_TAKAO_BASE_HPP


#include <M5Unified.h>
#include "Stackchan_servo.h"

enum PowerStatus {
    SidePower,
    BackPower,
    Battery
};

uint8_t checkTakaoBasePowerStatus(m5::Power_Class* power, StackchanSERVO* servo ) {
  bool last_ext_output = power->getExtOutput();
  if (power->Axp192.getACINVoltage() > 1.0f) {
    power->setExtOutput(true);
    power->setLed(80);
    return PowerStatus::SidePower;  
  }
  while (servo->isMoving()) {delay(1);} // サーボが動いている間は待機（そうしないとサーボの動きが乱れる。）
  power->setExtOutput(false); // 後側のUSB-Cの状態を把握するためにfalseにする必要があります。
  delay(500);
  if (power->Axp192.getBatteryDischargeCurrent() > 3.0f) {
    power->setExtOutput(true);
    power->setLed(0);
    return PowerStatus::Battery;
  }
  power->setExtOutput(false);
  power->setLed(80);
  return PowerStatus::BackPower; 
}

#endif // STACKCHAN_TAKAO_BASE_HPP