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

void setup() {
  setupPrinter(GBIn, GBOut, GBClock);
  initPrinter();
  Serial.begin(9600);
  delay(200);
  Serial.println("Gameboy Printer for Arduino");
}

void loop() {
  
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
// Data Out flush row data set 4 times
  if(cmd == 'g') {
    Serial.println("Init");        
    sendInitialize();
    getStatusCode();

    for (int j=0; j<4; j++) {
      // each row is started like this
      sendRow(hadl);
      // and ends like this
    }
    
    Serial.println("Inq");        
    sendInquiry();
    
    Serial.println("Data"); 
    GBPCommand(GBData,0);
    printStatusCode(-1);
    Serial.println("Print");
    sendPrint(1,3,0xE4,0x40);
    printStatusCode(-1);
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
  }
  
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