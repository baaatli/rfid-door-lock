/*

   Manual:
   To wipe all the cards, hold WIPEBUTTON and press RESET button. Keep WIPEBUTTON holded for 10 seconds
   To add or remove card, hold WIPEBUTTON and press RESET button, immediately release WIPEBUTTON, scan card, already existing cards will be removed, new cards will be added to memory.
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
#define wipeButton 3
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

//reads new card and stores UID in readCard variable. waits until a card is scanned.
bool readRFID() {
  //  while (!(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()));      //wait till a new card is present
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return true;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return true;
  }
  Serial.println("card detected");
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  return false;
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

bool deleteID(byte findThis[]) {
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i < count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if (compare(findThis, storedCard)) {   // Check to see if the storedCard is present in EEPROM
      uint8_t start = (i * 4) + 2;
      uint8_t looping = ((count - i) * 4);
      count--;
      EEPROM.write(0, count);  // Write the new count to the counter
      for ( uint8_t j = 0; j < looping; j++ ) {         // Loop the card shift times
        EEPROM.write(start + j, EEPROM.read(start + 4 + j));   // Shift the array values to 4 places earlier in the EEPROM
      }
      for ( uint8_t k = 0; k < 4; k++ ) {         // clearing the end
        EEPROM.write( start + looping + k, 0);
      }
      Serial.println(F("Succesfully removed ID record from EEPROM"));
      return true;
    }
  }
  Serial.println("Nothing to delete, ID not found");
  return false;
}

void addID(byte addThis[]) {
  if ( !findID( addThis ) ) {                   // Before we write to the EEPROM, check to see if we have seen this card before!
    Serial.print("Succesfully added ID record: ");
    Serial.println(EEPROM.read(0));
    uint8_t num = EEPROM.read(0);               // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t start = ( num * 4 ) + 2;            // Figure out where the next slot starts
    num++;                                      // Increment the counter by one
    EEPROM.write( 0, num );                     // Write the new count to the counter
    for ( uint8_t j = 0; j < 4; j++ ) {         // Loop 4 times
      Serial.print("/////////////////");
      Serial.println(start);
      Serial.println(j);
      EEPROM.write( start + j, addThis[j] );    // Write the array values to EEPROM in the right position
      Serial.print(addThis[j]);
    }
  }
  else {
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
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
  pinMode(wipeButton, INPUT_PULLUP);
  pinMode(relay, OUTPUT);
  //  pinMode(RST_PIN, INPUT);

  if (digitalRead(wipeButton) == LOW) {
    unsigned int startTime = millis();
    while (digitalRead(wipeButton) == LOW);
    unsigned int endTime = millis();
    Serial.println(endTime - startTime);
    if (endTime - startTime < 1000) {            //single press
      Serial.println("******************\nSuperUser mode\n******************\n \nIf a known card is scanned, it will be deleted \nIf an unknown card is scanned, it will be added as a key");
      Serial.print("Keys I have: ");
      Serial.println(EEPROM.read(0));
      while (readRFID());                              //stores UID to readCard variable
      if (deleteID(readCard)) {
        Serial.println("Card deleted");
      }
      else {
        addID(readCard);
      }
    }
    else if (endTime - startTime > 5000) {      // super long press
      //Erase EEPROM if wipebutton is holded for 5 seconds
      for (uint16_t x = 0; x < EEPROM.length(); x = x + 1) {    //Loop end of EEPROM address
        if (EEPROM.read(x) == 0) {              //If EEPROM address 0
          // do nothing, already clear, go to the next address in order to save time and reduce writes to EEPROM
        }
        else {
          EEPROM.write(x, 0);       // if not write 0 to clear, it takes 3.3mS
        }
      }
      Serial.println("wiped everything");
    }
  }

  mfrc522.PCD_Init();   // Initiate MFRC522
  delay(5);
  Serial.println("Put your card to the reader...");
}


void loop()
{
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_NoCmdChange);
  delay(1000);
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
  }

  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_NoCmdChange | 0x10);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
