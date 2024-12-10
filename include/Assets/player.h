#include <avr/pgmspace.h>
#include "st7735Helper.h"

/*
0 = black
1 = light brown 
2 = green 
3 = blue
4 = blue-grey 
5 = yellow 
6 = white
7 = gray
8 = light blue
*/
const unsigned char facingUp_sprite[16][16] PROGMEM = {
  {6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
  {6,6,6,6,6,6,0,0,0,0,6,6,6,6,6,6},
  {6,6,6,6,6,0,0,0,0,0,0,6,6,6,6,6},
  {6,6,6,6,6,0,0,0,0,0,0,6,6,6,6,6},
  {6,6,6,6,6,0,0,0,0,0,0,6,6,6,6,6},
  {6,6,6,6,6,0,0,0,0,0,0,6,6,6,6,6},
  {6,6,6,6,6,6,0,0,0,0,6,6,6,6,6,6},
  {7,6,6,6,2,2,2,2,2,2,2,2,6,6,6,7},
  {7,7,6,6,2,2,2,2,2,2,2,2,6,6,7,7},
  {7,7,7,6,2,2,2,2,2,2,2,2,6,7,7,7},
  {7,7,7,7,2,2,2,2,2,2,2,2,7,7,7,7},
  {7,7,7,7,1,1,3,3,3,3,1,1,7,7,7,7},
  {7,7,7,7,7,7,3,3,3,3,7,7,7,7,7,7},
  {0,0,0,0,0,0,3,3,3,3,0,0,0,0,0,0},
  {0,0,0,0,0,0,3,3,3,3,0,0,0,0,0,0},
  {0,0,0,0,0,0,4,4,4,4,0,0,0,0,0,0}
};
const unsigned char facingDown_sprite[16][16] PROGMEM = {
  {0,0,0,0,0,7,7,7,7,7,7,0,0,0,0,0},
  {0,0,0,0,7,7,0,0,0,0,7,7,0,0,0,0},
  {7,7,7,7,7,0,0,1,1,0,0,7,7,7,7,7},
  {7,7,7,7,7,0,1,1,1,1,0,7,7,7,7,7},
  {7,7,7,7,7,0,1,1,1,1,0,7,7,7,7,7},
  {7,7,7,7,7,0,1,1,1,1,0,7,7,7,7,7},
  {7,7,7,7,7,7,0,1,1,0,7,7,7,7,7,7},
  {7,7,7,7,2,2,2,2,2,2,2,2,7,7,7,7},
  {7,7,7,7,2,2,2,2,2,2,2,2,7,7,7,7},
  {7,7,7,7,2,2,2,2,2,2,2,2,7,7,7,7},
  {7,7,7,7,2,2,2,2,2,2,2,2,7,7,7,7},
  {7,7,7,7,0,1,3,3,3,3,1,1,7,7,7,7},
  {7,7,7,7,5,6,3,3,3,3,6,6,7,7,7,7},
  {7,7,7,6,6,6,3,3,3,3,6,6,6,7,7,7},
  {7,7,6,6,6,6,3,3,3,3,6,6,6,6,7,7},
  {7,6,6,6,6,6,4,4,4,4,6,6,6,6,6,7}
};
const unsigned char facingLeft_sprite[16][16] PROGMEM = {
  {8,7,7,7,7,7,7,7,7,7,7,7,0,0,0,0},
  {8,8,7,7,7,0,0,0,0,0,7,7,7,0,0,0},
  {6,8,7,7,7,0,1,1,0,0,0,7,7,7,0,0},
  {6,8,7,7,7,7,1,1,0,0,0,7,7,7,0,0},
  {6,8,7,7,7,7,1,1,1,1,0,7,7,7,0,0},
  {6,8,7,7,7,7,1,1,1,0,0,7,7,7,0,0},
  {6,8,7,7,7,7,7,1,1,1,7,7,7,7,0,0},
  {6,8,7,7,7,7,2,2,2,2,2,2,7,7,0,0},
  {6,8,7,7,7,7,2,2,2,2,2,2,7,7,0,0},
  {6,5,0,0,1,2,2,2,2,2,2,2,7,7,0,0},
  {6,8,7,7,1,2,2,2,2,2,2,2,7,7,0,0},
  {6,8,7,7,7,7,7,3,3,3,1,1,7,7,0,0},
  {6,8,7,7,7,7,7,3,3,3,3,7,7,7,0,0},
  {6,8,7,7,7,7,7,3,3,3,3,7,7,7,0,0},
  {8,8,7,7,7,7,7,3,3,3,3,7,7,0,0,0},
  {8,7,7,7,7,7,4,4,4,4,4,7,0,0,0,0}
};
const unsigned char facingRight_sprite[16][16] PROGMEM = {
  {0,0,0,0,7,7,7,7,7,7,7,7,7,7,7,8},
  {0,0,0,7,7,7,0,0,0,0,0,7,7,7,8,8},
  {0,0,7,7,7,0,0,0,1,1,0,7,7,7,8,6},
  {0,0,7,7,7,0,0,0,1,1,7,7,7,7,8,6},
  {0,0,7,7,7,0,1,1,1,1,7,7,7,7,8,6},
  {0,0,7,7,7,0,0,1,1,1,7,7,7,7,8,6},
  {0,0,7,7,7,7,1,1,1,7,7,7,7,7,8,6},
  {0,0,7,7,2,2,2,2,2,2,7,7,7,7,8,6},
  {0,0,7,7,2,2,2,2,2,2,7,7,7,7,8,6},
  {0,0,7,7,2,2,2,2,2,2,2,1,0,0,5,6},
  {0,0,7,7,2,2,2,2,2,2,2,1,7,7,8,6},
  {0,0,7,7,1,1,3,3,3,7,7,7,7,7,8,6},
  {0,0,7,7,7,3,3,3,3,7,7,7,7,7,8,6},
  {0,0,7,7,7,3,3,3,3,7,7,7,7,7,8,6},
  {0,0,0,7,7,3,3,3,3,7,7,7,7,7,8,8},
  {0,0,0,0,7,4,4,4,4,4,7,7,7,7,7,8}
};

struct player{
    bool alive = true;
    int keys = 0;
    int direction = 1;    // 1: left, 2: right, 3: up, 4: down
    unsigned char scope_direction = direction;
};

void drawPlayer(unsigned char xStart, unsigned char xEnd, unsigned char yStart, unsigned char yEnd, player playerObject){
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

  uint8_t pixel;
  playerObject.scope_direction = (playerObject.direction!=0) ? playerObject.direction : playerObject.scope_direction;
  for (int i = 0; i < 16; i++){
    for (int j = 0; j < 16; j++){
      if(playerObject.scope_direction==1)
        pixel = pgm_read_byte(&facingLeft_sprite[i][j]);
      else if(playerObject.scope_direction==2)
        pixel = pgm_read_byte(&facingRight_sprite[i][j]);
      else if(playerObject.scope_direction==3)
        pixel = pgm_read_byte(&facingUp_sprite[i][j]);
      else if(playerObject.scope_direction==4)
        pixel = pgm_read_byte(&facingDown_sprite[i][j]);

      if(pixel==0){    // black
        Send_Data(0x00);
        Send_Data(0x00);
      }
      else if(pixel==1){    // light brown
        Send_Data(0x3d);
        Send_Data(0xdf);
      }
      else if(pixel==2){    // green
        Send_Data(0x0b);
        Send_Data(0x06);
      }
      else if(pixel==3){    // blue
        Send_Data(0xde);
        Send_Data(0x40);
      }
      else if(pixel==4){    // blue-grey
        Send_Data(0xcc);
        Send_Data(0x09);
      }
      else if(pixel==5){    // silver
        Send_Data(0xf3);
        Send_Data(0xf0);
      }
      else if(pixel==6){    // white
        Send_Data(0xff);
        Send_Data(0xff);
      }
      else if(pixel==7){
        Send_Data(0x63);
        Send_Data(0x10);
      }
      else if(pixel==8){
        Send_Data(0xe7);
        Send_Data(0x70);
      }
    }
  }
}

player playerOne;
unsigned char playerOne_coord[] = {7,7};
int scopeX=-1, scopeY=-1;
int old_scopeX, old_scopeY;
