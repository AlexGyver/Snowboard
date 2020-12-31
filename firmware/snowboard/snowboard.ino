/*
  Скетч к проекту "Симулятор сноуборда"
  Страница проекта (схемы, описания): https://alexgyver.ru/Snowboard/
  Исходники на GitHub: https://github.com/AlexGyver/Snowboard
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2020
  https://AlexGyver.ru/
*/

#define OUTPUT_TYPE 0
// 0 - gamepad
// 1 - serial for processing
// 2 - serial for plotter raw
// 3 - serial for plotter derive

#define COMPUTE_PERIOD 50 // период расчёта и отправки, мс

#define SIT_VALUE 3000    // порог распознавания приседания
#define JUMP_VALUE 10000  // порог распознавания прыжка
#define JUMP_DEB 500      // дебаунс прыжка, мс

// либа кнопки
#include "EncButton.h"
EncButton<3> btn;

// мини-либа датчика
#include "hx711.h"
HX711 FR(14, 15); // (data, clock)
HX711 BR(10, 16);
HX711 FL(6, 7);
HX711 BL(8, 9);

// либа HID
#if (OUTPUT_TYPE == 0)
#include "HID-Project.h"
#endif

// либа eeprom
#include <EEPROM.h>

// макрос
#define _inRange(x, a, b) (((x) > (a)) && ((x) < (b)))
#define EE_RESET 10   // код первого пуска

// структура для удобной отправки в процессинг
struct Weight {
  int FR;
  int BR;
  int FL;
  int BL;
  uint16_t valX = 4500;
  uint16_t valY = 4500;
};
Weight data, derive, prev;

// структура для записи епром
struct Calibration {
  long calFR = 0;
  long calBR = 0;
  long calFL = 0;
  long calBL = 0;
  int maxX = 750;
  int minX = -750;
  int maxY = 1500;
  int minY = -1500;
};
Calibration cal;

// прыжок
bool jumpFlag = 0;
uint32_t jumpTmr = 0;

void setup() {
  // первый старт, ставим флаг епром
  if (EEPROM.read(0) != EE_RESET) {
    EEPROM.write(0, EE_RESET);
    EEPROM.put(1, cal);
  }

  // читаем из епрома
  EEPROM.get(1, cal);

  // и возвращаем калибровку
  FR.setOffset(cal.calFR);
  BR.setOffset(cal.calBR);
  FL.setOffset(cal.calFL);
  BL.setOffset(cal.calBL);

#if (OUTPUT_TYPE > 0)
  Serial.begin(9600);
  while (!Serial);
#else
  Gamepad.begin();
#endif
}

void loop() {
  buttonTick();   // опрос кнопки
  getData();      // получаем данные

  static uint32_t tmr;
  if (millis() - tmr > COMPUTE_PERIOD) {
    tmr = millis();
    computeXY();    // считаем управление
    jump();         // считаем прыжок
    output();       // вывод
  }
}

// ======================================
void output() {
  if (millis() > 2000) {  // 2 секунды от старта на успокоение
#if (OUTPUT_TYPE == 0)
    Gamepad.xAxis(data.valX);   // -32768.. 32767 16 бит ось
    Gamepad.yAxis(data.valY);
    Gamepad.zAxis(jumpFlag ? -100 : 0);   // 8 бит ось
    Gamepad.write();
#elif (OUTPUT_TYPE == 1)
    Serial.write((byte*)&data, sizeof(data));
#elif (OUTPUT_TYPE == 2)
    Serial.print(data.FR); Serial.print(',');
    Serial.print(data.BR); Serial.print(',');
    Serial.print(data.FL); Serial.print(',');
    Serial.print(data.BL); Serial.println();
#else
    Serial.print(derive.FR); Serial.print(',');
    Serial.print(derive.BR); Serial.print(',');
    Serial.print(derive.FL); Serial.print(',');
    Serial.print(derive.BL); Serial.print(',');
    Serial.print(SIT_VALUE); Serial.print(',');
    Serial.print(-JUMP_VALUE); Serial.print(',');
    Serial.print(jumpFlag * 5000); Serial.println();
#endif
  }
}

void getData() {
  // получаем дату и делим на 256, высокая точность нам не надо
  data.FR = FR.getData() >> 8;
  data.BR = BR.getData() >> 8;
  data.FL = FL.getData() >> 8;
  data.BL = BL.getData() >> 8;
}

void computeXY() {
  // расчёт  направлений джойстика
  long valX = ((data.FR - data.FL) + (data.BR - data.BL)) / 2;
  long valY = ((data.BL - data.FL) + (data.BR - data.FR)) / 2;

  // преобразуем в  -32768.. 32767
  valX = map(valX, cal.minX, cal.maxX, -0x7FFF, 0x7FFF);
  valY = map(valY, cal.minY, cal.maxY, -0x7FFF, 0x7FFF);
  valX = constrain(valX, -0x7FFF, 0x7FFF);
  valY = constrain(valY, -0x7FFF, 0x7FFF);
  data.valX = valX;
  data.valY = valY;
}

void jump() {
  // ищем производные
  derive.FR = (data.FR - prev.FR) * 1000L / COMPUTE_PERIOD;
  derive.BR = (data.BR - prev.BR) * 1000L / COMPUTE_PERIOD;
  derive.FL = (data.FL - prev.FL) * 1000L / COMPUTE_PERIOD;
  derive.BL = (data.BL - prev.BL) * 1000L / COMPUTE_PERIOD;
  prev.FR = data.FR;
  prev.BR = data.BR;
  prev.FL = data.FL;
  prev.BL = data.BL;
  if (millis() > 2000) {  // 2 секунды от старта на успокоение
    // присели
    if ((derive.FR> SIT_VALUE) &&
        (derive.BR> SIT_VALUE) &&
        (derive.FL> SIT_VALUE) &&
        (derive.BL> SIT_VALUE) &&
        !jumpFlag && millis() - jumpTmr > JUMP_DEB) {
      jumpFlag = 1;
      jumpTmr = millis();
      // встали
    } else if ((derive.FR < -JUMP_VALUE) &&
               (derive.BR < -JUMP_VALUE) &&
               (derive.FL < -JUMP_VALUE) &&
               (derive.BL < -JUMP_VALUE) &&
               jumpFlag && millis() - jumpTmr > JUMP_DEB) {
      jumpFlag = 0;
      jumpTmr = millis();
    }
  }
}

void buttonTick() {
  btn.tick();
  if (btn.isClick()) {    // клик
    // калибровка
    FR.calibrate();
    BR.calibrate();
    FL.calibrate();
    BL.calibrate();

    // получаем значение
    cal.calFR = FR.getOffset();
    cal.calBR = BR.getOffset();
    cal.calFL = FL.getOffset();
    cal.calBL = BL.getOffset();
    // и в епром его
    EEPROM.put(1, cal);
  }
  if (btn.isHolded()) {   // удержание
    uint32_t tmr = millis();
    cal.maxX = -32000;
    cal.minX = 32000;
    cal.maxY = -32000;
    cal.minY = 32000;
    while (millis() - tmr < 10000) {  // в теч 10 секунд
      // читаем
      data.FR = FR.getData() >> 8;
      data.BR = BR.getData() >> 8;
      data.FL = FL.getData() >> 8;
      data.BL = BL.getData() >> 8;
      long valX = ((data.FR - data.FL) + (data.BR - data.BL)) / 2;
      long valY = ((data.BL - data.FL) + (data.BR - data.FR)) / 2;
      // ищем пределы
      cal.maxX = max(valX, cal.maxX);
      cal.maxY = max(valY, cal.maxY);
      cal.minX = min(valX, cal.minX);
      cal.minY = min(valY, cal.minY);
    }
    EEPROM.put(1, cal); // сохраняем
  }
}
