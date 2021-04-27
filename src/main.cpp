#include <MFRC522.h> // for the RFID
#include <SPI.h>     // for the RFID and SD card module
#include <SD.h>      // for the SD card
#include <RTClib.h>  // for the RTC
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define CS_RFID 10
#define RST_RFID 9
#define CS_SD 4

LiquidCrystal_I2C lcd(0x27, 16, 2);
File myFile;
MFRC522 rfid(CS_RFID, RST_RFID);
String uidString;
RTC_DS1307 rtc;

const int buzzer = 5;
const int sensorflow = 3;
const int potensio = A0;

const int pompa = 6;

void setup()
{
  pinMode(buzzer, OUTPUT);
  pinMode(sensorflow, INPUT);

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
  // inisialisasi lcd disini
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("smart water");
  lcd.setCursor(0, 1);
  lcd.print("start");
}

void readRFID()
{
  rfid.PICC_ReadCardSerial();
  Serial.print("Tag UID: ");
  uidString = String(rfid.uid.uidByte[0]) + " " + String(rfid.uid.uidByte[1]) + " " +
              String(rfid.uid.uidByte[2]) + " " + String(rfid.uid.uidByte[3]);
  Serial.println(uidString);

  lcd.setCursor(0, 0);
  lcd.print("tag UID :             ");
  lcd.setCursor(0, 1);
  lcd.print(uidString);
  lcd.print("        ");

  tone(buzzer, 2000);
  delay(100);
  noTone(buzzer);
  delay(100);
}

void logCard()
{
  int nilaiair = analogRead(potensio);
  nilaiair = map(nilaiair, 1024, 0, 0, 5000);
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
    myFile.print(now.minute(), DEC);
    myFile.print(',');
    myFile.println(nilaiair);

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
  int volumeair = analogRead(potensio);
  volumeair = map(volumeair, 1024, 0, 5000, 0);

  lcd.setCursor(0, 0);
  lcd.print("volume air :          ");
  lcd.setCursor(0, 1);
  lcd.print(volumeair);
  lcd.print("                ");

  //look for new cards
  if (rfid.PICC_IsNewCardPresent())
  {
    readRFID();
    logCard();
    delay(1000);
    if (digitalRead(sensorflow) == HIGH)
    {
      digitalWrite(pompa, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("air panas        ");
      delay(volumeair);
    }
    else
    {
      digitalWrite(pompa, LOW);
    }
  }
  digitalWrite(pompa,LOW);
  delay(200);
}
