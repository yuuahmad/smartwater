#include <MFRC522.h> // for the RFID
#include <SPI.h>     // for the RFID and SD card module
#include <SD.h>      // for the SD card
#include <RTClib.h>  // for the RTC
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define CS_RFID 10
#define RST_RFID 9
#define CS_SD 4

LiquidCrystal_I2C lcd(0x27, 20, 4);
File myFile;
MFRC522 rfid(CS_RFID, RST_RFID);
String uidString;
RTC_DS1307 rtc;

const int buzzer = 5;
const int motorpanas = 6;
const int motordingin = 7;
const int sensorpanas = 2;
const int sensordingin = 3;
const int potensio = A0;

void setup()
{
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
  while (!Serial)
    ; // for Leonardo/Micro/Zero
  SPI.begin();
  rfid.PCD_Init();
  Serial.print("Initializing SD card...");
  if (!SD.begin(CS_SD))
  {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }
  else
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  if (!rtc.isrunning())
  {
    Serial.println("RTC is NOT running!");
  }
}

void readRFID()
{
  rfid.PICC_ReadCardSerial();
  Serial.print("Tag UID: ");
  uidString = String(rfid.uid.uidByte[0]) + " " + String(rfid.uid.uidByte[1]) + " " +
              String(rfid.uid.uidByte[2]) + " " + String(rfid.uid.uidByte[3]);
  Serial.println(uidString);
  tone(buzzer, 2000);
  delay(100);
  noTone(buzzer);
  delay(100);
}

void logCard()
{
  digitalWrite(CS_SD, LOW);
  myFile = SD.open("DATA.txt", FILE_WRITE);
  if (myFile)
  {
    Serial.println("File opened ok");
    myFile.print(uidString);
    myFile.print(", ");

    // Save time on SD card
    DateTime now = rtc.now();
    myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(',');
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.println(now.minute(), DEC);

    // Print time on Serial monitor
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.println(now.minute(), DEC);
    Serial.println("sucessfully written on SD card");
    myFile.close();
  }
  else
  {
    Serial.println("error opening data.txt");
  }
  // Disables SD card chip select pin
  digitalWrite(CS_SD, HIGH);
}

void loop()
{
  //look for new cards
  if (rfid.PICC_IsNewCardPresent())
  {
    readRFID();
    logCard();
  }
  delay(10);
}
