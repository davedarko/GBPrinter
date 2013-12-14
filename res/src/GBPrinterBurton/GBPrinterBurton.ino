/* This is based on the works of Miles Burton and the furrtek gang/guy.
 * I made this a library for easier integration.
 * 
 * Almost the original pde from Miles Burton, only extracted the library files
 * http://www.milesburton.com/Gameboy_Printer_with_Arduino
 */

#include <GBPrinter.h>

void setup() {
  setupPrinter(GBIn, GBOut, GBClock);
  Serial.begin(9600);
  Serial.println("Gameboy Printer for Arduino");
}

uint8_t buffer[64];

//uint8_t c,b,cmd,repl,buffer[64];
uint8_t cmd;
uint16_t CRC;

void loop() {
  //serialCommand = Serial.read();
  cmd = Serial.read();

  // connection Test
  if (cmd == '?') {

    if (sendInitialize()) {
      getStatusCode();
      Serial.print('1');
    } 
    else {
      Serial.print('0');
    }
  }

// Blackbar Test
  if(cmd == 'g') {
    Serial.println("Init");        
    sendInitialize();
    getStatusCode();
    CRC = 0;
    Serial.println("Data");
    CRC += beginData();

    for(int i=0; i<640; ++i) {
      GBSerialOut(0xFF);
    }

    if(endData(CRC)) //0x27E06
    { 
      Serial.println("Data sent");
    }
    printStatusCode(-1);

    Serial.println("Inq");        
    sendInquiry();
    Serial.println("Data");    
    GBPCommand(GBData,0);
    printStatusCode(-1);
    Serial.println("Print");
    sendPrint(1,3,0xE4,0x40);
    printStatusCode(-1);

    // sendInquiry();
  }

  // Manage printing
  if (cmd == '!') {
    GBSerialOut(0x88);
    GBSerialOut(0x33);
    GBSerialOut(0x01);
    GBSerialOut(0x00);
    GBSerialOut(0x00);
    GBSerialOut(0x00);
    GBSerialOut(0x01);
    GBSerialOut(0x00);
    getAcknowledgement();
    //SerialPut(repl);
    getStatusCode();
    //SerialPut(repl);

    GBSerialOut(0x88);
    GBSerialOut(0x33);
    GBSerialOut(0x04);
    GBSerialOut(0x00);
    GBSerialOut(0x80);
    GBSerialOut(0x02);
    CRC = 0x86;

    for (int b=0;b<10;b++) {
      Serial.print('a');

      for (int c=0;c<64;c++) {
        buffer[c] = Serial.read();
      }

      for (int c=0;c<64;c++) {
        cmd = buffer[c];
        GBSerialOut(cmd);
        CRC += cmd;
      }
    }

    GBSerialOut((uint8_t)CRC & 0xFF);			// CRC Low
    GBSerialOut((CRC & 0xFF00)>>8);				// CRC High
    getAcknowledgement();
    //SerialPut(repl);
    getStatusCode();
    //SerialPut(repl);



    delay(10); 		// 131: répond 4 au lieu de 8, 265 timeout (0)

    GBSerialOut(0x88);	// Data vide
    GBSerialOut(0x33);
    GBSerialOut(0x04);
    GBSerialOut(0x00);
    GBSerialOut(0x00);
    GBSerialOut(0x00);
    GBSerialOut(0x04);
    GBSerialOut(0x00);
     getAcknowledgement();
    //SerialPut(repl);
    getStatusCode();
    //SerialPut(repl);

    GBSerialOut(0x88);	// Print
    GBSerialOut(0x33);
    GBSerialOut(0x02);
    GBSerialOut(0x00);
    GBSerialOut(0x04);
    GBSerialOut(0x00);

    CRC = 6;
    Serial.print('1');
    for (int c=0;c<4;c++) {
      buffer[c] = Serial.read();
    }

    for (int c=0;c<4;c++) {
      cmd = buffer[c];
      GBSerialOut(cmd);
      CRC += cmd;
    }

    GBSerialOut((uint8_t)CRC & 0xFF);			// CRC Low
    GBSerialOut((CRC & 0xFF00)>>8);				// CRC High

    getAcknowledgement();
    //SerialPut(repl);
    getStatusCode();
    //SerialPut(repl);

    while(1) {
      GBSerialOut(0x88);	// Status
      GBSerialOut(0x33);
      GBSerialOut(0x0F);
      GBSerialOut(0x00);
      GBSerialOut(0x00);
      GBSerialOut(0x00);
      GBSerialOut(0x0F);
      GBSerialOut(0x00);
      GBSerialOut(0);
      if (GBSerialOut(0) & 2) break;
      delay(100);
    }
    Serial.print('P');

    while(1) {
      GBSerialOut(0x88);	// Status
      GBSerialOut(0x33);
      GBSerialOut(0x0F);
      GBSerialOut(0x00);
      GBSerialOut(0x00);
      GBSerialOut(0x00);
      GBSerialOut(0x0F);
      GBSerialOut(0x00);
      GBSerialOut(0);
      if (!(GBSerialOut(0) & 2)) break;
      delay(100);
    }
    Serial.print('O');

  }
  // End Printing
}
