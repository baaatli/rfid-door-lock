/*

   Manual:
   No wipe button
   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

   1kb EEPROM on ATMEGA328P
*/

#include <SPI.h>
#include <EEPROM.h>     // We are going to read and write PICC's UIDs from/to EEPROM
#include <MFRC522.h>
#include <LowPower.h>

#define SS_PIN 10
#define RST_PIN 8
#define relay 5

byte readCard[4];
byte storedCard[4];

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}

//returns true if given key is present in EEPROM
bool findID(byte findThis[]) {
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 0; i < count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if (compare(findThis, storedCard)) {   // Check to see if the storedCard read from EEPROM
      return true;
    }
  }
  return false;
}

bool compare (byte a[], byte b[]) {
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
      return false;
    }
  }
  return true;
}

void setup()
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();          // Initiate  SPI bus
  pinMode(relay, OUTPUT);
  //  pinMode(RST_PIN, INPUT);

  mfrc522.PCD_Init();   // Initiate MFRC522
  delay(5);
  Serial.println("Put your card to the reader...");
}


void loop()
{
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_NoCmdChange);
  delay(1000);
  for (int counter = 0; counter < 4 ; counter++) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
    {
      Serial.print("UID tag :");

      for ( uint8_t i = 0; i < 4; i++) {  //
        readCard[i] = mfrc522.uid.uidByte[i];
        Serial.print(readCard[i], HEX);
      }

      if (findID(readCard)) {
        Serial.println("Access Granted");
        digitalWrite(relay, HIGH);
        delay(3000);
        digitalWrite(relay, LOW);
      }
      else {
        Serial.println("Access Denied");
      }
      break;
    }
  }
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_NoCmdChange | 0x10);
  delay(1000);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
