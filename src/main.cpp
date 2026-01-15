#include <Arduino.h>
#include "BluetoothSerial.h"
#include <BleKeyboard.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#include "esp_sleep.h"

RTC_DATA_ATTR int bootCount = 0;

BluetoothSerial SerialBT;
BleKeyboard bleKeyboard("ESP32 Keyboard", "ESP32", 100);

#define EEPROM_SIZE 512   // Размер EEPROM (байт), достаточно для малого JSON
#define JSON_MAX_SIZE 200 // Максимальный размер JSON-строки
// Массив пинов для кнопок
/*inMode(32,INPUT_PULLUP);
  pinMode(33,INPUT_PULLUP); 
  pinMode(25,INPUT_PULLUP);
  pinMode(26,INPUT_PULLUP);
  pinMode(27,INPUT_PULLUP);
  pinMode(14,INPUT_PULLUP);
  pinMode(12,INPUT_PULLUP);
  pinMode(13,INPUT_PULLUP); */
const int buttonPins[] = {32, 33, 25,26,27,14,12,13};
const int numButtons = 8;
const int buttonPinMode = 13;
const int buttonPinModeSleep = 12;
int bt[8];



// Возможные клавиши для выбора в веб-интерфейсе

 const MediaKeyReport * keyOptionsMedia[]  = 
{
&KEY_MEDIA_NEXT_TRACK,
&KEY_MEDIA_PREVIOUS_TRACK,
&KEY_MEDIA_STOP,
&KEY_MEDIA_PLAY_PAUSE,
&KEY_MEDIA_MUTE ,
&KEY_MEDIA_VOLUME_UP,
&KEY_MEDIA_VOLUME_DOWN,
&KEY_MEDIA_CALCULATOR,
&KEY_MEDIA_WWW_SEARCH,
&KEY_MEDIA_WWW_STOP,
&KEY_MEDIA_WWW_BACK,
&KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION
  
};
String  keyOptionsMediaString[]  = 
{
"KEY_MEDIA_NEXT_TRACK",
"KEY_MEDIA_PREVIOUS_TRACK",
"KEY_MEDIA_STOP",
"KEY_MEDIA_PLAY_PAUSE",
"KEY_MEDIA_MUTE" ,
"KEY_MEDIA_VOLUME_UP",
"KEY_MEDIA_VOLUME_DOWN",
"KEY_MEDIA_CALCULATOR",
"KEY_MEDIA_WWW_SEARCH",
"KEY_MEDIA_WWW_STOP",
"KEY_MEDIA_WWW_BACK",
"KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION"
  
};
const u_int8_t * keyOptionsKS[] =
 {
  &KEY_LEFT_CTRL,
  &KEY_LEFT_SHIFT,
  &KEY_LEFT_ALT, 
  &KEY_LEFT_GUI
};
int mode = 0; // режим работы

// Функция сохранения JSON в EEPROM
bool saveJsonToEEPROM(String jsonStr)
{
  if (jsonStr.length() >= EEPROM_SIZE - 1)
  { // -1 для завершающего нуля
    return false;
  }

  // Запись длины строки (2 байта)
  uint16_t length = jsonStr.length();
  EEPROM.write(0, (uint8_t)(length & 0xFF)); // Младший байт
  EEPROM.write(1, (uint8_t)(length >> 8));   // Старший байт

  // Запись строки
  for (uint16_t i = 0; i < length; i++)
  {
    EEPROM.write(2 + i, jsonStr[i]);
  }
  // Завершающий нуль
  EEPROM.write(2 + length, 0);

  EEPROM.commit(); // Сохранение изменений
  return true;
}


     
    
// Функция чтения JSON из EEPROM
String readJsonFromEEPROM()
{
  // Чтение длины строки
  uint16_t length = (EEPROM.read(1) << 8) | EEPROM.read(0);
  if (length >= EEPROM_SIZE - 2)
  { // Проверка корректности длины
    return "";
  }

  // Чтение строки
  char buffer[JSON_MAX_SIZE];
  for (uint16_t i = 0; i < length; i++)
  {
    buffer[i] = EEPROM.read(2 + i);
  }
  buffer[length] = '\0'; // Завершающий нуль
  return String(buffer);
}

void setup()
{
  Serial.begin(115200);
  /*while (!Serial)
  {
    ; // Ожидание инициализации Serial
  }*/
  Serial.println("ESP32 готов ");
  EEPROM.begin(EEPROM_SIZE);
  pinMode(buttonPinMode, INPUT_PULLUP);
   pinMode(buttonPinModeSleep, INPUT_PULLUP);
  mode = !digitalRead(buttonPinMode);
 
  if (!digitalRead(buttonPinModeSleep)==1)
  {
    mode = 2;
  }
delay(100);
   Serial.println("режим ");
   Serial.println(!digitalRead(buttonPinModeSleep));
   Serial.println( mode);
  delay(100);

   switch (mode)
  {
  case 0: // режим работы
    Serial.println("режим работы ");
    // Настройка пинов кнопок как входов с подтягивающим резистором
    for (int i = 0; i < numButtons; i++)
    {
      pinMode(buttonPins[i], INPUT_PULLUP);
    }
    

    {
      String writeJson = readJsonFromEEPROM();

      writeJson.trim();
      if (writeJson.length() > 0)
      {
        Serial.println("JSON из EEPROM: " + writeJson);
        // Парсинг входящего JSON
        JsonDocument doc; // Используем JsonDocument вместо StaticJsonDocument
        DeserializationError error = deserializeJson(doc, writeJson);

        if (error)
        {
          Serial.println("Ошибка парсинга: ");
          Serial.println(error.c_str());

          return;
        }
        Serial.println("парсинг yes: ");
        
        
        bt[0] =  doc["bt1"];
        bt[1] = doc["bt2"];
        bt[2] =  doc["bt3"];
        bt[3] = doc["bt4"];
        bt[4] =  doc["bt5"];
        bt[5] = doc["bt6"];
        bt[6] =  doc["bt7"];
        bt[7] = doc["bt8"];
        Serial.printf("bt1 = %d" , bt[0]);
        Serial.println();
        Serial.printf("bt2 = %d" , bt[1]);
         Serial.println();
        
       

        bleKeyboard.begin(); // Инициализация BluetoothKeyboard
        Serial.println("Starting BLE Keyboard with Web Interface!");
      }
    }
    break;

  case 1:                            // режим конфигурации
    SerialBT.begin("ESP32_BT_JSON"); // Имя Bluetooth-устройства
    Serial.println("ESP32 готов к конфигурации по Bluetooth. Отправьте JSON!");
    Serial.println("режим конфигурации");
    break;
    
    case 2:                            // режим сна
  Serial.print("Сон ");
  delay(100);
  esp_deep_sleep_start();
    break;
  
    
  }
 
}
 


void loop()
{

  switch (mode)
  {
  case 0: // режим работы
    while (true)
    {
     if (bleKeyboard.isConnected())
      {
        for (int i = 0; i < numButtons; i++)
        {
          if (!digitalRead(buttonPins[i]))
          {

            switch (i)
            {
            case 0: 
                    bleKeyboard.write(*keyOptionsMedia[bt[0]]);
              Serial.println("Пин 32 "+ keyOptionsMediaString[bt[0]] );
              break;
            case 1: 
                    bleKeyboard.write(*keyOptionsMedia[bt[1]]);//0x0c
              Serial.println("Пин 33 "+ keyOptionsMediaString[bt[1]]);
              break;
              case 2: 
                    bleKeyboard.write(*keyOptionsMedia[bt[2]]);
              Serial.println("Пин 25 " +keyOptionsMediaString[bt[2]]);
              break;
            case 3: 
                    bleKeyboard.write(*keyOptionsMedia[bt[3]]);
              Serial.println("Пин 26 "+ keyOptionsMediaString[bt[3]]);
              break;
              case 4: 
                    bleKeyboard.write(*keyOptionsMedia[bt[4]]);
              Serial.println("Пин 27 "+ keyOptionsMediaString[bt[4]]);
              break;
            case 5:
                    bleKeyboard.write(*keyOptionsMedia[bt[5]]);
              Serial.println("Пин 14 "+ keyOptionsMediaString[bt[5]]);
              break;
              case 6: 
                    bleKeyboard.write(*keyOptionsMedia[bt[6]]);
              Serial.println("Пин 12 "+ keyOptionsMediaString[bt[6]]);
              break;
            case 7:
                    bleKeyboard.write(*keyOptionsMedia[bt[7]]);
              Serial.println("Пин 13 "+ keyOptionsMediaString[bt[7]]);
              break;
            }
            delay(100);               // Короткая задержка для антидребезга
            bleKeyboard.releaseAll(); // Отпускаем все клавиши
          }
        }
      }

    
    
      delay(100);
    }

    break;

  case 1: // режим конфигурации
  
  while (true)
  {
   
    if (SerialBT.available())
   //if (SerialBT.connected())
    {
      
     /* String ConnectData = SerialBT.readStringUntil('\n');
      ConnectData.trim();  
      String writeJson = readJsonFromEEPROM();
      writeJson.trim();
      Serial.println(ConnectData);
      if (ConnectData == "Connected")
      {
     
      Serial.println("Serial.println подключение установлено");
      SerialBT.println("подключение установлено");

      
      
     
      }
      else if(ConnectData == "setOptions")
      {
        
        SerialBT.println(writeJson);
         Serial.println("Serial.println ошибка проверки");
          Serial.println("Serial.println  получено " + ConnectData);
      }*/
      
      
     // String po= "{"bt1":1,"bt2":2,"bt3":1,"bt4":2,"bt5":1,"bt6":2,"bt":1,"bt8":2}";
      String receivedData = SerialBT.readStringUntil('\n'); // Чтение до \n
      receivedData.trim();                                  // Удаление пробелов/переносов

      if (receivedData.length() > 0)
      {
        Serial.println("Получено: " + receivedData);

        // Парсинг входящего JSON
        JsonDocument doc; // Используем JsonDocument вместо StaticJsonDocument
        DeserializationError error = deserializeJson(doc, receivedData);

        if (error)
        {
          Serial.print("Ошибка парсинга: ");
          Serial.println(error.c_str());
          SerialBT.println("Ошибка JSON!");
          return;
        }

        // Извлечение значений
       bt[0] = doc["bt1"];
       bt[1] = doc["bt2"];
        Serial.printf("bt1 = " + bt[0], "bt2= " + bt[1]);

        // Сохранение JSON в EEPROM
        if (saveJsonToEEPROM(receivedData))
        {
          Serial.println("JSON успешно сохранён в EEPROM" + receivedData);
        }
        else
        {
          Serial.println("Ошибка: JSON слишком большой для EEPROM");
        }

        // Отправка подтверждения
        JsonDocument response; // Используем JsonDocument для ответа
        response["status"] = "ok";
        response["received"] = receivedData;
        serializeJson(response, SerialBT);
        SerialBT.println();
      }
    }
  }
    break;

}
}
