#ifndef GBPrinter_h
#define GBPrinter_h

#include <avr/pgmspace.h>
#include "Arduino.h"

#define delayMs 20
#define GBClock  8
#define GBIn  9
#define GBOut  10

// Command codes
#define GBInitialize  0x01
#define GBData 0x04
#define GBPrint 0x02
#define GBInquiry 0x0f

#define MarginNone 0x0
#define MarginMin 0x1
#define MarginMax 0xF

#define GBStatus_LowBattery 0x80
#define GBStatus_TempBad 0x40
#define GBStatus_Jam 0x20
#define GBStatus_Busy 0x10
#define GBStatus_Good 0x00

// GBIn needs 15kohm pullup (5v).
void setupPrinter(int in, int out, int clock);


void printStatusCode(int result);


// 8 clock intervals (8bits per packet)
uint8_t GBSerialOut(uint8_t command);

/*
A command packet consists of the following four parts: a Synchronization mark, a Header, a Body, and a Checksum, in this order.
 */

// Send a command packet to GB Pritner
boolean GBPCommand(uint8_t command, uint16_t bodyLength);

// This sequence indiciates the beginning of a command packet
void sendSync();

/*
A Header consists of four bytes. 
 The first byte in the Header is a command code, and the second is a compression indicator.
 Third and fourth bytes forms a 16 bit integer (in LSB-first byte order, i.e., the third byte is the lower 8 bits and the fourth the higher) 
 representing the length of the Body of the command packet in bytes. 
 This value may be zero to indicate an empty Body. 
 */
uint16_t sendHeader(uint8_t commandCode, uint8_t compressionIndicator, uint16_t bodyLength);

/*
A Checksum is a two-byte (16 bit) value to verify a command packet is received properly.
 It is always appended at the end of a command packet. The Checksum is a 16 bit arithmetic 
 sum over bytes in the command packet, excluding the Synchronization marker (i.e., 0x83 and 0x33 at the beginning of the packet) 
 and the Checksum, regarding each byte as an eight-bit unsigned integer. 
 */

void sendChecksum(uint16_t checksum);

/*
GB sends one Initialize command when it starts printing a page. If it attempts to print several pages in sequence, one Initialize command is sent for each page. 
 
 Unlike other commands, a Print command requires long time to complete, since the command requires some mechanical actions to complete.
 GB games that print usually issue Inquiry commands periodically after a Print command to wait for the command to complete. 
 */
boolean sendInitialize();

/*
A command packet whose command code is set to 0x0F is an Inquiry command. The compression indicator of the Inquiry command is set to zero (0x00.) 
 The length field is always set to zero (0x00 0x00,) to indicate the Body in the command is empty. The purpose of Inquiry command is to make GB Printer to notify its status to the GB. So, this command is suitable to be used after a Print command or before an Initialize command. 
 However, unlike other commands, Inquiry command may be issued at any time. The GB Printer is expected to respond to an Inquiry command always. 
 
 */
uint8_t sendInquiry();

/*
A plain printing Data command is used to transmit (a part of) printing image from GB to Printer. 
 The compression indicator in Header is set to zero (0x00.) The length field in Header is set to 0x280, or 640 in decimal. 
 Since 16 bit values are transmitted in LSB-first byte order, it is actually transmitted as 0x80 0x02.
 The Body in plain printing Data command represents a band (160x16 dots or 20x2 tiles) of printing image in plain (uncompressed) format. 
 */
uint16_t beginData();

/*
One empty Data command is sent after the last printing Data command for a page and before the Print command for the page. 
 The purpose of this command is unknown. 
 */
boolean endData(uint16_t checksum);


// Any value between 0x1 and 0xF is for some margin between minimum and maximum, apparently proportional to the specified value. 
/*
  A Print command is issued after sufficient number of printing Data commands have been issued. 
 When the Print command is received, the GB Printer physically starts printing. 
 In other words, printing image data sent between the last Initialize command and this Print command collectively form a page. 
 */
boolean sendPrint(uint8_t leftMargin, uint8_t rightMargin, uint8_t palette, uint8_t exposure);


/*
A band consists of 40 tiles. A tile is an image consisting of 8x8 dots. Color of each dot is specified by a two-bit value, so a dot can be a color out of four. Actually, GB Printer prints only greyscale (monochrome) image, so possible colors are: white, light grey, dark grey, and black. A tile is represented by 16 bytes of data. The format of a tile is exactly same as that of GB display tile pattern, such as background tile, window tile, or sprite tile. 
 */

// A band consists of two strips (2 strips, 20x strip. 40 tiles)
void sendBand();

// 160 pixels wide / 8 = 20 tiles
void sendStrip();

// A tile consists of 16 bytes
// 8x8
void sendTestTile();

/*
An acknowledgement code is set (by GB Printer) to either 0x80 or 0x81. 
 The difference of those two values is unsure at this moment. 
 */
boolean getAcknowledgement();

/*
  A status code is a bitmap to indicate various Printer statuses. It has bit-by-bit meanings.
 */
uint8_t getStatusCode();

#endif