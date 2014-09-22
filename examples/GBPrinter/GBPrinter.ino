/* This is based on the works of Miles Burton and the furrtek gang/guy.
 * I made this a library for easier integration into sketches
 * TODO: 
 * TextToPrint ( thats what the alphabet array is for)
 * 
 * 
 * Byte1 0x0101 0101
 * Byte2 0x0011 0011
 * color   0123 0123
 * 
 * http://www.milesburton.com/Gameboy_Printer_with_Arduino
 */
 
#include <GBPrinter.h>

//uint8_t c,b,cmd,repl,buffer[64];
uint8_t cmd;
uint16_t CRC;
uint8_t buffer[64];
/*
  copy the hex output from http://davedarko.com/gameboy.php here
  but make sure your arduino really got the byte size memory
  I "bricked" a fake arduino uploading a script that was supposed to be 
  smaller then the max amount.
*/
uint8_t row0[640];
uint8_t row1[640];
uint8_t row2[640];
uint8_t row3[640];
uint8_t row4[640];
uint8_t row5[640];
uint8_t row6[640];
uint8_t row7[640];

void setup() {
  setupPrinter(GBIn, GBOut, GBClock);
  initPrinter();
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Gameboy Printer for Arduino");
}

void loop() {
  if (Serial.available() > 0) {

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

    if (cmd == 'A') {
      printText("A");  
    }

    if(cmd == 'h') {
      hadCon();
    }

    if (cmd=='d') {
      ddimg(); 
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

      GBSerialOut((uint8_t)CRC & 0xFF);     // CRC Low
      GBSerialOut((CRC & 0xFF00)>>8);       // CRC High
      getAcknowledgement();
      //SerialPut(repl);
      getStatusCode();
      //SerialPut(repl);



      delay(10);    // 131: répond 4 au lieu de 8, 265 timeout (0)

      GBSerialOut(0x88);  // Data vide
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

      GBSerialOut(0x88);  // Print
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

      GBSerialOut((uint8_t)CRC & 0xFF);     // CRC Low
      GBSerialOut((CRC & 0xFF00)>>8);       // CRC High

      getAcknowledgement();
      //SerialPut(repl);
      getStatusCode();
      //SerialPut(repl);

      while(1) {
        GBSerialOut(0x88);  // Status
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
        GBSerialOut(0x88);  // Status
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
  }
}

void sendRow (uint8_t row2send[]) {
    CRC = 0;
    Serial.println("Data");
    CRC += beginData();
    for(int i=0; i<640; ++i) {
      uint8_t cmd = row2send[i];
      CRC += cmd;
      GBSerialOut(cmd);
    }

    if(endData(CRC)) //0x27E06
    { 
      Serial.println("Data sent");
    }
    printStatusCode(-1);
}

// prints stored logo
void hadCon () {
  Serial.println("Init");        
  sendInitialize();
  getStatusCode();
  sendRow(hadl);
  
  Serial.println("Inq");        
  sendInquiry();
  
  Serial.println("Data"); 
  GBPCommand(GBData,0);
  printStatusCode(-1);
  Serial.println("Print");
  sendPrint(1,3,0xE4,0x40);
  printStatusCode(-1);
}

void ddimg () {
  Serial.println("Init");        
  sendInitialize();
  getStatusCode();
  
  //uncomment this if your arduino has the power
  /*
  sendRow(row0);
  sendRow(row1); 
  sendRow(row2); 
  sendRow(row3);
  sendRow(row4);
  sendRow(row5);
  sendRow(row6);
  sendRow(row7);  
  /**/
  Serial.println("Inq");        
  sendInquiry();
  
  Serial.println("Data"); 
  GBPCommand(GBData,0);
  printStatusCode(-1);
  Serial.println("Print");
  sendPrint(1,3,0xE4,0x40);
  printStatusCode(-1);
}

// Check for shorted RX/TX
void initPrinter() {
  int TX = 1;
  int RX = 0;
  int buttonState = 0;
  pinMode(RX, INPUT);   // RX    
  pinMode(TX, OUTPUT);  // TX
  digitalWrite(TX, LOW); 
  buttonState = digitalRead(RX);
  delay(200);
  if (buttonState == LOW) hadCon();
}

// cleartext to printer command
void printText (String c) {
  uint8_t printRow[640];
  for (int iletter = 0; iletter<c.length(); iletter++) {
    uint8_t car = c[iletter];
    
    if (car>31 && car<127) {
/*      Serial.write(car);
      Serial.print(": ");
      Serial.println(car, DEC);
      
       for (int  i= 0; i<5; i++) {
        Serial.print((car-32)*5+i);
        Serial.print("\t");
      }
      Serial.println();
      
  */    
      for (int  i= 0; i<5; i++) {
//        Serial.print((car-31)*5+i);
 //       Serial.print(": ");
      //  uint8_t b = alphabet[(car-32)*5+i];
//        Serial.print(b, HEX);
     //   print_binary(b, 8);
        Serial.println();
      }
    //  Serial.println();
      Serial.println();

    }
  }
}