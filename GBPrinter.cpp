#include "GBPrinter.h"
#include "Arduino.h"
#include <avr/pgmspace.h>

// GBIn needs 15kohm pullup (5v).
void setupPrinter(int in, int out, int clock) {
  pinMode(out, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(in, INPUT);           // set pin to input
  digitalWrite(in, HIGH);       // turn on pullup resistors
  digitalWrite(out, HIGH);       // turn on pullup resistors
}

void printStatusCode(int result)
{
  if(result<0)
  {
    result = getStatusCode();
  }
  /*
  switch(result)
  {
  case 0:
   // Serial.println("Stat: Good");
    break; 
  case 0x08:
   // Serial.println("Stat: Received OK");
    break;
  default:
  //  Serial.println("Stat: Error? - Dec");
  //  Serial.println(result,DEC);
  }*/

}


// 8 clock intervals (8bits per packet)
uint8_t GBSerialOut(uint8_t command) {
  uint8_t gbReplyy=0;
  for (uint8_t clock=0;  clock<8;  ++clock) {
    gbReplyy <<= 1;			// Shift Printer gbReplyy
    digitalWrite(GBClock, 0);           // Send clock signal

    if((command << clock) & 0x80){
      digitalWrite(GBOut, 1); // Write out to printer
    }
    else{ 
      digitalWrite(GBOut, 0);
    }

    delayMicroseconds(delayMs);		// Wait 20-60us
    digitalWrite(GBClock, 1);            // End clock
    if(digitalRead(GBIn))
    {
      gbReplyy |= 1;    // Fetch printer gbReplyy
    }
    delayMicroseconds(delayMs);		// Wait 20-60us
  }

  delayMicroseconds(delayMs);		// Wait 20-60us


    //Serial.print(command, HEX);
  //Serial.print(' ');
 // Serial.print(gbReplyy, HEX);
 // Serial.print('\n');

  return gbReplyy;

}

/*
A command packet consists of the following four parts: a Synchronization mark, a Header, a Body, and a Checksum, in this order.
 */

// Send a command packet to GB Pritner
boolean GBPCommand(uint8_t command, uint16_t bodyLength) {

  boolean result = false;
  sendSync();
  sendHeader(command, 0x00, bodyLength); 


  // 0x88 // Magic
  // 0x33 // Magic
  // 0x01 // Command
  // 0x00 // Length
  // 0x00 //Length
  // 0x01 // Sum of everything bar magic packet

  sendChecksum(bodyLength + command); // GBPCommand CRC
  result = getAcknowledgement();

  return result;
}

// This sequence indiciates the beginning of a command packet
void sendSync()
{
  // Magic Bytes
  GBSerialOut(0x88);	//10001000			
  GBSerialOut(0x33);    //00110011
}

/*
A Header consists of four bytes. 
 The first byte in the Header is a command code, and the second is a compression indicator.
 Third and fourth bytes forms a 16 bit integer (in LSB-first byte order, i.e., the third byte is the lower 8 bits and the fourth the higher) 
 representing the length of the Body of the command packet in bytes. 
 This value may be zero to indicate an empty Body. 
 */
uint16_t sendHeader(uint8_t commandCode, uint8_t compressionIndicator, uint16_t bodyLength)
{
  GBSerialOut(commandCode);            // 0000 xxxx
  GBSerialOut(compressionIndicator);   // 0000 000x
  // LSB (send as two bytes)
  // 0000 0000
  // 0000 0000
  GBSerialOut(bodyLength); // Lower 8 bits
  GBSerialOut(bodyLength>>8); // Higher 8 bits

  return commandCode + compressionIndicator + (uint8_t)bodyLength + ((uint8_t)(bodyLength>>8));
}

/*
A Checksum is a two-byte (16 bit) value to verify a command packet is received properly.
 It is always appended at the end of a command packet. The Checksum is a 16 bit arithmetic 
 sum over bytes in the command packet, excluding the Synchronization marker (i.e., 0x83 and 0x33 at the beginning of the packet) 
 and the Checksum, regarding each byte as an eight-bit unsigned integer. 
 */

void sendChecksum(uint16_t checksum)
{
  // LSB (send as two bytes)
  // 0000 0000
  // 0000 0000  
  GBSerialOut(checksum); // Lower 8 bits
  GBSerialOut(checksum>>8); // Higher 8 bits
}

/*
GB sends one Initialize command when it starts printing a page. If it attempts to print several pages in sequence, one Initialize command is sent for each page. 
 
 Unlike other commands, a Print command requires long time to complete, since the command requires some mechanical actions to complete.
 GB games that print usually issue Inquiry commands periodically after a Print command to wait for the command to complete. 
 */
boolean sendInitialize()
{
  return GBPCommand(GBInitialize, 0);
}

/*
A command packet whose command code is set to 0x0F is an Inquiry command. The compression indicator of the Inquiry command is set to zero (0x00.) 
 The length field is always set to zero (0x00 0x00,) to indicate the Body in the command is empty. The purpose of Inquiry command is to make GB Printer to notify its status to the GB. So, this command is suitable to be used after a Print command or before an Initialize command. 
 However, unlike other commands, Inquiry command may be issued at any time. The GB Printer is expected to respond to an Inquiry command always. 
 
 */
uint8_t sendInquiry()
{
  GBPCommand(GBInquiry, 0);
  return getStatusCode();
}

/*
A plain printing Data command is used to transmit (a part of) printing image from GB to Printer. 
 The compression indicator in Header is set to zero (0x00.) The length field in Header is set to 0x280, or 640 in decimal. 
 Since 16 bit values are transmitted in LSB-first byte order, it is actually transmitted as 0x80 0x02.
 The Body in plain printing Data command represents a band (160x16 dots or 20x2 tiles) of printing image in plain (uncompressed) format. 
 */
uint16_t beginData() // returns checksum
{
  sendSync();
  return sendHeader(GBData, 0x00, 0x280); // Grab checksum
}

/*
One empty Data command is sent after the last printing Data command for a page and before the Print command for the page. 
 The purpose of this command is unknown. 
 */
boolean endData(uint16_t checksum)
{
  sendChecksum(checksum);
  return getAcknowledgement();
}

boolean endPage()
{
  GBPCommand(GBData, 0);
  return getStatusCode() == 0x08;
}


// Any value between 0x1 and 0xF is for some margin between minimum and maximum, apparently proportional to the specified value. 
/*
  A Print command is issued after sufficient number of printing Data commands have been issued. 
 When the Print command is received, the GB Printer physically starts printing. 
 In other words, printing image data sent between the last Initialize command and this Print command collectively form a page. 
 */
boolean sendPrint(uint8_t leftMargin, uint8_t rightMargin, uint8_t palette, uint8_t exposure)
{
  uint16_t CRC = 0;
  sendSync();
  CRC += sendHeader(GBPrint, 0x00, 0x04); // Body in a Print command is always four bytes in length.

  CRC += 0x01;
  GBSerialOut(0x01); // The first byte is always set to 0x01, and its purpose is unknown. 


  // The second byte represents margins before and after the page
  // ----- The margins byte (the second byte in Body) is divided into two four-bit values.
  GBSerialOut((leftMargin << 4) + rightMargin);



  // The third byte defines the palette to be used to render the page
  GBSerialOut(palette); // Palette (11100100: Noir, Foncé, Clair, Blanc)

  // The fourth byte represents the dense of ink for printing (or the heat at the printing head since real GB Printer is a thermal prin
  // er.) 
  GBSerialOut(exposure);// Expo (7 bits)

  CRC += ((leftMargin << 4) + rightMargin) + palette + exposure;

  sendChecksum(CRC);

  return getAcknowledgement();
}


/*
A band consists of 40 tiles. A tile is an image consisting of 8x8 dots. Color of each dot is specified by a two-bit value, so a dot can be a color out of four. Actually, GB Printer prints only greyscale (monochrome) image, so possible colors are: white, light grey, dark grey, and black. A tile is represented by 16 bytes of data. The format of a tile is exactly same as that of GB display tile pattern, such as background tile, window tile, or sprite tile. 
 */

// A band consists of two strips (2 strips, 20x strip. 40 tiles)
void sendBand()
{
  for(int i=0; i<2; ++i) sendStrip();
}

// 160 pixels wide / 8 = 20 tiles
void sendStrip()
{
  for(int i=0; i<20; ++i) sendTestTile();
}

// A tile consists of 16 bytes
// 8x8
void sendTestTile()
{
  for(int i=0; i<16;++i)
  {
    GBSerialOut(0xFF);
  }
}

/*
An acknowledgement code is set (by GB Printer) to either 0x80 or 0x81. 
 The difference of those two values is unsure at this moment. 
 */
boolean getAcknowledgement()
{
  uint8_t ack = GBSerialOut(0); // Wait for confirmation
  return (ack==0x80 || ack == 0x81); // Good otherwise timeout
}

/*
  A status code is a bitmap to indicate various Printer statuses. It has bit-by-bit meanings.
 */
uint8_t getStatusCode()
{
  return GBSerialOut(0); // Wait for confirmation  
}








