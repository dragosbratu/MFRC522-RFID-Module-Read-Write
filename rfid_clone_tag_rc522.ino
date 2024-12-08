/*
* Use it as you wish https://github.com/dragosbratu
*/

#include <SPI.h>
#include <MFRC522.h>
#include <string.h>

#define RST_PIN         9    // Reset pin
#define SS_PIN          10   // Slave Select pin

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;     // Authentication key

byte clonedData[16];         // Buffer to store data to clone
bool hasDataToClone = false; // Flag to check if data is ready

void setup() {
  Serial.begin(9600);
  SPI.begin();               // Initialize SPI
  rfid.PCD_Init();           // Initialize RFID reader

  Serial.println("RFID Cloning Program");
  Serial.println("Type '0' for help menu.");
  Serial.println("1. Read a tag");
  Serial.println("2. Write (clone) a tag");
  
  // Default key for MIFARE Classic
  for (byte i = 0; i < 6; i++) 
  {
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
  if (Serial.available()) {
    char choice = Serial.read();
    switch (choice) {
      case '0': showHelpMenu(); break;
      case '1': readTag(); break;
      case '2': writeTag(); break;
      default: Serial.println("Invalid choice! Please enter 0, 1, or 2."); break;
    }
  }
}

void showHelpMenu() 
{
String MoreHelp = F("RFID tags operate on specific frequencies. Common frequencies are:\n"
                    "13.56 MHz (used by MIFARE and similar cards, compatible with RC522).\n"
                    "125 kHz (used by older proximity cards, not compatible with RC522).\n"
                    "The RC522 reader supports only 13.56 MHz RFID tags. If your second tag operates at 125 kHz, the RC522 module won't detect or read it.\n"
                    "Solution:\n"
                    "Check the specifications of the second tag. If it operates at 125 kHz, you will need a different reader, such as the EM-18 RFID reader, which supports 125 kHz.\n");

  Serial.println("\n=== Help Menu https://github.com/dragosbratu ===");
  Serial.println("This program is designed for cloning RFID tags.");
  Serial.println("You can perform the following actions:");
  Serial.println("1. Read a tag to extract its UID and data (block 4).");
  Serial.println("2. Write the extracted data to another writable tag.");
  Serial.println("The program assumes both tags are MIFARE Classic.");
  Serial.println("\nPin Mapping for Arduino Uno:");
  Serial.println("RC522 Pin   ->   Arduino Pin");
  Serial.println("SDA (SS)    ->   Pin 10");
  Serial.println("SCK         ->   Pin 13");
  Serial.println("MOSI        ->   Pin 11");
  Serial.println("MISO        ->   Pin 12");
  Serial.println("IRQ         ->   Not connected");
  Serial.println("GND         ->   GND");
  Serial.println("RST         ->   Pin 9");
  Serial.println("3.3V        ->   3.3V (do NOT use 5V)");
  Serial.println(MoreHelp);
  Serial.println("====================\n");
}

void readTag() {
  Serial.println("Bring the tag close to the reader...");
  while (!rfid.PICC_IsNewCardPresent()) {}  // Wait for a tag
  while (!rfid.PICC_ReadCardSerial()) {}    // Wait for valid tag read

  // Print UID
  Serial.print("Tag UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Authenticate and read data from block 4
  byte block = 4;
  byte buffer[18];
  byte size = sizeof(buffer);
  MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid));
  if (status == MFRC522::STATUS_OK) {
    status = rfid.MIFARE_Read(block, buffer, &size);
    if (status == MFRC522::STATUS_OK) {
      Serial.print("Data in block 4: ");
      for (byte i = 0; i < 16; i++) {
        clonedData[i] = buffer[i];
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      hasDataToClone = true;
    } else {
      Serial.println("Failed to read data from the block.");
    }
  } else {
    Serial.println("Authentication failed.");
  }

  rfid.PICC_HaltA(); // Halt communication
}

void writeTag() {
  if (!hasDataToClone) {
    Serial.println("No data to clone. Please read a tag first.");
    return;
  }

  Serial.println("Bring the writable tag close to the reader...");
  while (!rfid.PICC_IsNewCardPresent()) {}  // Wait for a tag
  while (!rfid.PICC_ReadCardSerial()) {}    // Wait for valid tag read

  // Authenticate and write data to block 4
  byte block = 4;
  MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid));
  if (status == MFRC522::STATUS_OK) {
    status = rfid.MIFARE_Write(block, clonedData, 16);
    if (status == MFRC522::STATUS_OK) {
      Serial.println("Data successfully written to the tag.");
    } else {
      Serial.println("Failed to write data to the tag.");
    }
  } else {
    Serial.println("Authentication failed.");
  }

  rfid.PICC_HaltA(); // Halt communication
}
