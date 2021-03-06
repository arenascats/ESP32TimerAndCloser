
/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */

 //DAT3-IO5  CMD-IO23 CLK-IO18 DAT0-IO19
#include "FS.h"
#include "SD.h"
#include "SPI.h"

void SDGetButton(){
    
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

//    listDir(SD, "/", 0);
//    createDir(SD, "/mydir");
//    listDir(SD, "/", 0);
//    removeDir(SD, "/mydir");
//    listDir(SD, "/", 2);
//    writeFile(SD, "/hello.txt", "Hello ");
//    appendFile(SD, "/hello.txt", "World!\n");
//    readFile(SD, "/hello.txt");
//    deleteFile(SD, "/foo.txt");
//    renameFile(SD, "/hello.txt", "/foo.txt");
//    readFile(SD, "/foo.txt");
//    testFileIO(SD, "/test.txt");
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

  
int getSDSize()
{
    SD.begin();
  return SD.totalBytes()/ (1024 * 1024);
}
int getSDUsed()
{
    SD.begin();
  return SD.usedBytes()/ (1024 * 1024);
}

double getSDPercent()
{
   SD.begin();
  return SD.usedBytes()/SD.totalBytes();
}

void sdend()
{
    SD.end();
}
