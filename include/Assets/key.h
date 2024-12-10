#include <avr/pgmspace.h>
#include "st7735Helper.h"

/*
0 = white
1 = yellow
*/
const unsigned char key_sprite[16][16] PROGMEM = {
    {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0},
    {0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0},
    {0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

void drawKey(unsigned char xStart, unsigned char xEnd, unsigned char yStart, unsigned char yEnd, const unsigned char sprite[16][16]){
  Send_Command(0x2a);    // CASET: column address set
  // Start X 
  Send_Data(0);     // high/upper byte
  Send_Data(xStart);     // low/lower byte
  // End X
  Send_Data(0);     // high/upper byte
  Send_Data(xEnd);   // low/lower byte

  Send_Command(0x2b);   // RASET: row address set
  // Start Y
  Send_Data(0);     // high/upper byte
  Send_Data(yStart);     // low/lower byte
  // End Y
  Send_Data(0);     // high/upper byte
  Send_Data(yEnd);   // low/lower byte
  
  Send_Command(0x2c);    // RAMWR: memory write 
  // Green bits 2-0 w/ Blue bits 4-0
  // Red bits 5-0 w/ Green bits 5-3
  for (int i = 0; i < 16; i++){
    for (int j = 0; j < 16; j++){
      uint8_t pixel = pgm_read_byte(&sprite[i][j]);
      if(pixel==0){
        Send_Data(0xff);
        Send_Data(0xff);
      }
      if(pixel==1){
        Send_Data(0x0e);
        Send_Data(0xff);
      }
    }
  }
}

unsigned char key1_coord[] = {5,6};
unsigned char key2_coord[] = {0,3};
unsigned char key3_coord[] = {5,1};
