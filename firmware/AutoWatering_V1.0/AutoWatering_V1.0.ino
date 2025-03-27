/*
  ------ОДНОКАНАЛЬНЫЙ АВТОПОЛИВ------

  Разработано: https://github.com/DIY-Elecron1cs
  Управление, настройка, схема: https://github.com/DIY-Elecron1cs/AutoWatering (репозиторий проекта)
*/

/*---пины---*/
#define SENSOR A0
#define PUMP 9

#define RTC_CLK 4
#define RTC_DATA 5
#define RTC_RST 3

#define ENC_CLK 7
#define ENC_DT 8
#define ENC_SW 6

/*---настройки---*/
#define PUMP_PWM 220
#define RELAY 1
#define PUMP_PULSE_PERIOD 1500
#define PUMP_MINUTES 5
#define _LCD_TYPE 1
#define RESET_CLOCK 0   //1 - установить время
#define WEEK_DAY 4      //текущий день недели
#define DEBUG 0         //для отладки в мониторе порта

/*---библиотеки---*/
#include <buildTime.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <GyverEncoder.h>
#include <virtuabotixRTC.h>

/*---объекты---*/
LiquidCrystal_I2C lcd(0x27, 16, 2);
Encoder enc(ENC_CLK, ENC_DT, ENC_SW);
virtuabotixRTC myRTC(RTC_CLK, RTC_DATA, RTC_RST);


/*---переменные---*/
uint8_t screenMode = 0;
bool pumpState = 0;
bool setFlag = 0;
uint8_t wateringDay = 1;
uint8_t wateringTime = 12;
uint8_t humidity;
uint8_t humidityLevel = 50;
uint32_t pumpPulseTimer;
bool pumpPulseFlag = 0;


/*---битмапы---*/
byte drop[8] = {
  0b00100, 0b00100, 0b01110, 0b01110, 0b11111, 0b11101, 0b11011, 0b01110
};

byte pump1[8] = {
  0b11111, 0b11011, 0b11111, 0b00100, 0b00100, 0b01110, 0b01010, 0b01110
};

byte pump2[8] = {
  0b00000, 0b11111, 0b11011, 0b11111, 0b00100, 0b01110, 0b01010, 0b01110
};

byte pump3[8] = {
  0b00000, 0b00000, 0b11111, 0b11011, 0b11111, 0b01110, 0b01010, 0b01110
};

byte underline[8] = {
  0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000
};

void setup() {
#if DEBUG == 1
  Serial.begin(9600);
#endif

#if RESET_CLOCK == 1
  myRTC.setDS1302Time(BUILD_SEC, BUILD_MIN, BUILD_HOUR, WEEK_DAY, BUILD_DAY, BUILD_MONTH, BUILD_YEAR);  //установка времени
#endif

  pinMode(SENSOR, INPUT);
  pinMode(PUMP, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.createChar(1, drop);
  lcd.createChar(2, pump1);
  lcd.createChar(3, pump2);
  lcd.createChar(4, pump3);
  lcd.createChar(5, underline);

  enc.setType(TYPE2);

  readEEPROM();
}

void loop() {
  myRTC.updateTime();
  enc.tick();
  switch (screenMode) {   //выбор экрана
    case 0:
      screen0();
      break;
    case 1:
      screen1();
      break;
    case 2:
      screen2();
      break;
    case 3:
      screen3();
      break;
  }

  if (enc.isRight() && setFlag != 1) {    //если повернули энкодер вправо
#if DEBUG == 1
    Serial.println("Режим +");
#endif
    lcd.clear();
    screenMode = screenMode + 1;          //прибавляем счётчик режимов
    if (screenMode >= 3) {
      screenMode = 3;
    }
  }
  if (enc.isLeft() && setFlag != 1) {
#if DEBUG == 1
    Serial.println("Режим -");
#endif
    lcd.clear();
    screenMode = screenMode - 1;                //уменьшаем счётчик режимов
    if (screenMode <= 0 || screenMode > 3) {    //если повернули энкодер влево
      screenMode = 0;
    }
  }
  humidity = map(analogRead(SENSOR), 0, 1024, 0, 100);
  watering();
}

void screen0() {      //функция нулевого экрана
  static uint32_t pumpRotationTimer;
  static uint8_t pumpRotationState;
  String dispTime = String(myRTC.hours) + ":" + String(myRTC.minutes) + ":" + String(myRTC.seconds) + "   ";  //строка для вывода на дисплей
  lcd.setCursor(0, 0);    //выводим
  lcd.print("Time: ");
  lcd.print(dispTime);
  lcd.setCursor(0, 1);
  lcd.print("\1 ");
  lcd.print(humidity);
  lcd.print("%  ");

  lcd.setCursor(8, 1);
  if (pumpState == 1) {   //если помпа включена
    if (millis() - pumpRotationTimer >= 200) {    //анимация работы помпы
      pumpRotationTimer = millis();
      pumpRotationState = pumpRotationState + 1;
      if (pumpRotationState >= 4) {
        pumpRotationState = 0;
      }
    }
    switch (pumpRotationState) {
      case 0:
        lcd.print("\2");
        break;
      case 1:
        lcd.print("\3");
        break;
      case 2:
        lcd.print("\4");
        break;
      case 3:
        lcd.print("\3");
        break;
    }
  }
  else {
    lcd.print("\2");
  }
  if (pumpState == 1) {   //выводим текущее состояние помпы
    lcd.print(" ON   ");
  }
  else {
    lcd.print(" OFF  ");
  }
}

void screen1() {      //функция первого экрана
  static bool pointer = 0;
  if (setFlag == 0) {           //если НЕ в режиме настройки
    lcd.setCursor(0, pointer);  //рисуем указатель слева
    lcd.print(">");
    lcd.setCursor(0, !pointer);
    lcd.print(" ");

    lcd.setCursor(15, pointer);
    lcd.print(" ");
  }
  else {                        //если в режиме настройки
    lcd.setCursor(15, pointer); //рисуем указатель справа
    lcd.print("<");
    lcd.setCursor(15, !pointer);
    lcd.print(" ");

    lcd.setCursor(0, pointer);
    lcd.print(" ");
  }

  enc.tick();
  if (enc.isClick()) {
    pointer = !pointer;
  }
  if (enc.isHolded()) {
    setFlag = !setFlag;
  }
  lcd.setCursor(2, 0);
  lcd.print("Days: ");
  lcd.print(wateringDay);
  lcd.print("  ");
  lcd.setCursor(2, 1);
  lcd.print("Time: ");
  lcd.print(wateringTime);
  lcd.print("  ");
  if (setFlag == 1) {     //если режим настройки
    if (pointer == 0) {   //и если указател на первой строке
      enc.tick();
      if (enc.isRight()) {              //если крутим вправо
        wateringDay = wateringDay + 1;  //увеличиваем параметр
        if (wateringDay >= 5) {
          wateringDay = 5;
        }
      }
      if (enc.isLeft()) {               //если крутим влево
        wateringDay = wateringDay - 1;  //уменьшаем параметр
        if (wateringDay <= 1 || wateringDay > 5) {
          wateringDay = 1;
        }
      }
    }
    if (pointer == 1) {   //и если указател на второй строке
      enc.tick();
      if (enc.isRight()) {                //если крутим вправо
        wateringTime = wateringTime + 1;  //увеличиваем параметр
        if (wateringTime >= 23) {
          wateringTime = 23;
        }
      }
      if (enc.isLeft()) {                 //если крутим влево
        wateringTime = wateringTime - 1;  //уменьшаем параметр
        if (wateringTime <= 0 || wateringTime > 23) {
          wateringTime = 0;
        }
      }
    }
  }
}

void screen2() {          //функция второго экрана
  if (setFlag == 0) {     //рисуем указатель
    lcd.setCursor(15, 1);
    lcd.print(" ");
  }
  else {
    lcd.setCursor(15, 1);
    lcd.print("<");
  }
  lcd.setCursor(0, 0);    //выводим значения
  lcd.print("\1 ");
  lcd.print(humidity);
  lcd.print("%  ");

  enc.tick();
  if (enc.isClick()) {
    setFlag = !setFlag;
  }
  if (setFlag == 1) {                     //если режим настройки
    if (enc.isRight()) {                  //если крутим вправо
      humidityLevel = humidityLevel + 1;  //увеличиваем параметр
      if (humidityLevel >= 100) {
        humidityLevel = 100;
      }
    }
    if (enc.isLeft()) {                   //если крутим вправо
      humidityLevel = humidityLevel - 1;  //уменьшаем параметр
      if (humidityLevel <= 0 || humidityLevel > 100) {
        humidityLevel = 0;
      }
    }
  }
  lcd.setCursor(0, 1);    //выводим значения
  lcd.print("Level: ");
  lcd.print(humidityLevel);
  lcd.print("%  ");
}

void screen3() {      //функция третьего экрана
  static bool pointer = 1;
  lcd.setCursor(0, 0);
  lcd.print("Save?");
  lcd.setCursor(8, 0);
  lcd.print("YES   NO");
  if (enc.isClick()) {
    pointer = !pointer;
  }
  if (pointer == 0) {           //если указатель слева
    lcd.setCursor(8, 1);
    lcd.print("\5\5\5     ");   //подчёркиваем YES
  }
  else {                        //если указатель права
    lcd.setCursor(8, 1);
    lcd.print("      \5\5");    //подчёркиваем NO
  }
  if (enc.isHolded() && pointer == 0) {
    saveEEPROM();               //сохраняем в EEPROM
    screenMode = 0;
    return;
  }
  if (enc.isHolded() && pointer == 1) {
    screenMode = 0;
    return;
  }
}

void saveEEPROM() {              //сохраняем в EEPROM
#if DEBUG == 1
  Serial.println("Сохранено в EEPROM");
#endif
  EEPROM.write(5, wateringDay);
  EEPROM.write(6, wateringTime);
  EEPROM.write(7, humidityLevel);
}

void readEEPROM() {              //читаем из EEPROM
#if DEBUG == 1
  Serial.println("Прочитаны данные из EEPROM");
#endif
  wateringDay = EEPROM.read(5);
  wateringTime = EEPROM.read(6);
  humidityLevel = EEPROM.read(7);
}

void watering() {         //полив
  if (wateringStatus(myRTC.dayofmonth, myRTC.month, wateringDay) == true && myRTC.hours == wateringTime && myRTC.minutes < PUMP_MINUTES) {    //если надо поливать
    if (humidity < humidityLevel) {     //если почва сухая
      if (millis() - pumpPulseTimer >= PUMP_PULSE_PERIOD) {
        pumpPulseTimer = millis();
        pumpWork(pumpPulseFlag);
        pumpPulseFlag = !pumpPulseFlag;
      }
      pumpState = 1;
#if DEBUG == 1
      Serial.println("Идёт полив...");
#endif
    }
    else {            //если почва влажная
      pumpWork(LOW);
      pumpState = 0;
#if DEBUG == 1
      Serial.println("Полив завершён.");
#endif
    }
  }
  else {
    pumpWork(LOW);    //выключаем помпу
    pumpState = 0;
  }
}

bool wateringStatus(int day, int month, int wateringFrequency) {      //проверяем, нужно ли сегодня поливать
  if (day < 1 || day > 31 || month < 1 || month > 12) {               //исключения
    return false;
  }
  int dayOfYear = day;
  int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};  //дни в месяцах
  for (int i = 1; i < month; i++) {
    dayOfYear += daysInMonth[i];      //определяем, какой сейчас день в году по счёту
  }
  return (dayOfYear % wateringFrequency == 0);    //проверяем делимость
}

void pumpWork(bool state) {
#if RELAY == 1
  digitalWrite(PUMP, state);
#else
  if (state == HIGH)
    analogWrite(PUMP, PUMP_PWM);
  else
    analogWrite(PUMP, 0);
#endif
}
