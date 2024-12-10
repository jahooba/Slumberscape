#include <avr/pgmspace.h>
#include "st7735Helper.h"

/*
0 = black
1 = darkest red
2 = dark red
3 = red
4 = white
*/
const unsigned char hunting_sprite[16][16] PROGMEM = {
    {4,1,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
    {1,4,1,1,4,4,4,3,3,4,4,4,4,1,1,1},
    {1,4,4,4,1,4,3,3,3,3,4,4,1,4,4,1},
    {4,1,4,4,1,3,3,3,3,3,3,4,1,4,1,1},
    {4,4,4,4,3,3,3,3,3,3,3,3,1,4,4,4},
    {4,4,4,3,3,3,3,3,3,3,3,3,3,4,4,4},
    {4,4,3,3,3,3,3,3,3,3,3,3,3,3,4,4},
    {4,3,3,3,2,2,0,0,4,4,2,2,3,3,3,4},
    {4,3,3,3,2,2,0,0,0,0,2,2,3,3,3,4},
    {4,4,3,3,3,2,2,0,0,2,2,3,3,3,4,4},
    {4,4,4,3,3,3,2,2,2,2,3,3,3,4,4,4},
    {4,4,4,1,3,3,3,3,3,3,3,3,1,1,4,4},
    {4,4,1,1,4,3,3,3,3,3,3,4,4,4,1,1},
    {4,1,1,4,4,4,3,3,3,3,4,4,4,4,4,1},
    {1,4,4,4,4,4,4,3,3,4,4,4,1,4,1,4},
    {4,1,1,4,4,4,4,4,4,4,4,4,4,1,1,4}
};

const unsigned char stunned_sprite[16][16] PROGMEM = {
    {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
    {4,4,4,1,4,4,4,3,3,4,4,4,4,4,4,4},
    {4,4,1,4,1,4,3,3,3,3,4,4,4,4,1,4},
    {4,1,1,4,1,3,3,3,3,3,3,4,1,1,4,1},
    {4,4,4,4,3,3,3,3,3,3,3,3,1,4,1,4},
    {4,4,4,3,3,3,3,3,3,3,3,3,3,4,4,4},
    {4,4,3,3,3,3,3,3,3,3,3,3,3,3,4,4},
    {4,3,3,3,1,3,3,3,3,3,3,1,3,3,3,4},
    {4,3,3,3,3,1,1,1,1,1,1,3,3,3,3,4},
    {4,4,3,3,1,3,3,3,3,3,3,1,3,3,4,4},
    {4,4,4,3,3,3,3,3,3,3,3,3,3,4,4,4},
    {4,4,4,1,3,3,3,3,3,3,3,3,1,1,4,4},
    {4,1,1,1,4,3,3,3,3,3,3,4,4,1,4,1},
    {4,4,1,4,4,4,3,3,3,3,4,4,1,4,1,4},
    {4,1,4,1,4,4,4,3,3,4,4,4,4,1,4,4},
    {4,4,1,4,4,4,4,4,4,4,4,4,4,4,4,4}
};

struct enemy{
    bool forward = true;
    bool stunned = false;
};

void drawEnemy(unsigned char xStart, unsigned char xEnd, unsigned char yStart, unsigned char yEnd, enemy enemyObject){
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
      uint8_t pixel = pgm_read_byte((!enemyObject.stunned) ? &hunting_sprite[i][j] : &stunned_sprite[i][j]);
      if(pixel==0){     // black
        Send_Data(0x00);
        Send_Data(0x00);
      }
      else if(pixel==1){    // darkest red
        Send_Data(0x00);
        Send_Data(0xd8);
      }
      else if(pixel==2){    // dark red
        Send_Data(0x00);
        Send_Data(0xf8);
      }
      else if(pixel==3){    // red
        Send_Data(0x00);   
        Send_Data(0x10);
      }
      else if(pixel==4){    // white
        Send_Data(0xff);
        Send_Data(0xff);
      }
    }
  }
}

enemy enemy_Left;
unsigned char enemy_Left_coord[] = {0,0};
int enemy_Left_moves; //8

enemy enemy_Middle;
unsigned char enemy_Middle_coord[] = {3,2};
int enemy_Middle_moves; //6

enemy enemy_Right;
unsigned char enemy_Right_coord[] = {3,6};
int enemy_Right_moves; //11