#include <EEPROM.h>
#include <Servo.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Change according to your LCD address

#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo ServoMotor;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = { A0, A1, A2, A3 };
byte colPins[COLS] = { 5, 4, 3, 2 };

Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String readRFID;
String okRFID_1 = "43e794f6"; // Change UID TAG of RFID card/tag
String okRFID_2 = "49151eb3"; // Change UID TAG of RFID card/tag

const String password_1 = "1234"; // Change password as desired (optional)
const String password_2 = "ABCD";  // Change password as desired (optional)
const String password_3 = "1111";  // Change password as desired (optional)

String inputPassword;

boolean RFIDMode = true;
char key = 0;
char appData;

int servoAngle;
String doorStatus;

unsigned long currentTime = millis();
unsigned long lcdTime = 0;
unsigned long lcdTimeout = 20000;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  ServoMotor.attach(6);
  inputPassword.reserve(10);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  servoAngle = EEPROM.read(0);
  ServoMotor.write(servoAngle);
}

void loop() {
  if (millis() >= lcdTime + lcdTimeout) {
    lcd.noBacklight();
  }
  while (Serial.available() > 0) {
    appData = Serial.read();
    delay(1000);
    if (appData == 'A') {
      UnlockDoor();
    }
    if (appData == 'B') {
      LockDoor();
    }
  }
  key = customKeypad.getKey();
  if (key) {
    lcd.backlight();
    currentTime = millis();
    lcdTime = currentTime;
    Serial.println(key);
    if (key == '0') {
      LockDoor();
    }
  }
  if (servoAngle == 35) {
    doorStatus = ("DOOR UNLOCKED");
  }
  if (servoAngle == 90) {
    doorStatus = ("   DOOR LOCKED");
  }

  if (RFIDMode == true) {
    lcd.setCursor(0, 0);
    lcd.print("SMART DOOR LOCK");
    lcd.setCursor(1, 1);
    lcd.print("<< DOOR STATUS >>");
    lcd.setCursor(0, 2);
    lcd.print(doorStatus);

    if (!mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
    if (!mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    dumpByteArray(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println(readRFID);

    if (readRFID == okRFID_1 || readRFID == okRFID_2) {
      lcd.clear();
      lcd.print("ACCESS GRANTED");
      lcd.setCursor(0, 1);
      lcd.print("PLEASE ENTER");
      lcd.setCursor(5, 2);
      lcd.print("PASSWORD");
      RFIDMode = false;
    }
    else {
      lcd.clear();
      lcd.print("ACCESS DENIED");
      lcd.setCursor(0, 1);
      lcd.print("YOUR CARD IS");
      lcd.setCursor(0, 2);
      lcd.print("NOT REGISTERED!!!!!");
      delay(5000);
      lcd.clear();
      RFIDMode = true;
    }
  }
  if (RFIDMode == false) {
    if (key) {
      Serial.println(key);
      if (key == '*') {
        inputPassword = "";
        lcd.clear();
        lcd.print("ACCESS GRANTED");
        lcd.setCursor(0, 1);
        lcd.print("PLEASE ENTER");
        lcd.setCursor(5, 2);
        lcd.print("PASSWORD");
      }
      else if (key == '#') {
        lcd.clear();
        if (inputPassword == password_1 || inputPassword == password_2 || inputPassword == password_3) {
          UnlockDoor();
          lcd.clear();
          RFIDMode = true;
        }
        else {
          lcd.setCursor(0, 0);
          lcd.print("WRONG PASSWORD");
          lcd.setCursor(0, 1);
          lcd.print("PLEASE TRY AGAIN");
          delay(2000);
          lcd.clear();
          RFIDMode = true;
        }

        inputPassword = "";
      }
      else {
        if (inputPassword.length() == 0) {
        }
        inputPassword += key;
        lcd.setCursor(inputPassword.length(), 3);
        lcd.print('*');
      }
    }
    if (!mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
    if (!mfrc522.PICC_ReadCardSerial()) {
      return;
    }
    dumpByteArray(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println(readRFID);

    if (readRFID == okRFID_1 || readRFID == okRFID_2) {
      lcd.clear();
      lcd.print("ACCESS GRANTED");
      lcd.setCursor(0, 1);
      lcd.print("PLEASE ENTER");
      lcd.setCursor(5, 2);
      lcd.print("PASSWORD");
      RFIDMode = false;
    }
    else {
      lcd.clear();
      lcd.print("ACCESS DENIED");
      lcd.setCursor(0, 1);
      lcd.print("YOUR CARD IS");
      lcd.setCursor(0, 2);
      lcd.print("NOT REGISTERED!!!!!");
      delay(5000);
      lcd.clear();
      RFIDMode = true;
    }
  }
}

void dumpByteArray(byte * buffer, byte bufferSize) {
  lcd.backlight();
  currentTime = millis();
  lcdTime = currentTime;
  readRFID = "";
  for (byte i = 0; i < bufferSize; i++) {
    readRFID = readRFID + String(buffer[i], HEX);
  }
}

void UnlockDoor() {
  lcd.backlight();
  currentTime = millis();
  lcdTime = currentTime;
  servoAngle = 35;
  ServoMotor.write(servoAngle);
  delay(500);
  EEPROM.write(0, servoAngle);
  delay(200);
  lcd.clear();
  lcd.print("DOOR UNLOCKED");
  lcd.setCursor(0, 1);
  lcd.print("PLEASE ENTER");
  lcd.setCursor(5, 2);
  lcd.print("PASSWORD");
  delay(5000);
  lcd.clear();
}

void LockDoor() {
  lcd.backlight();
  currentTime = millis();
  lcdTime = currentTime;
  servoAngle = 90;
  ServoMotor.write(servoAngle);
  delay(500);
  EEPROM.write(0, servoAngle);
  delay(200);
  lcd.clear();
  lcd.print("DOOR LOCKED");
  lcd.setCursor(0, 1);
  lcd.print("PLEASE ENTER");
  lcd.setCursor(5, 2);
  lcd.print("PASSWORD");
  delay(5000);
  lcd.clear();
}
