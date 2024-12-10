//#include <avr/pgmspace.h>
#include "st7735Helper.h"
#include "enemy.h"
#include "player.h"
#include "wall.h"
#include "key.h"
#include "exit.h"

bool flashlightPressed = false;   // determines if flashlight pressed
unsigned char exitLocks = 3;      // holds number of locks for game condition

const int MAZE_XSTART = 2;
const int MAZE_XEND = 129;
const int MAZE_YSTART = 1;
const int MAZE_YEND = 128;

const char startingMaze[8][8] = {   //[rows = j] [columns = i]
        {' ', ' ', ' ', 'k', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', 'w', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', 'w', ' ', 'w', 'w', ' '},
        {' ', 'n', ' ', 'w', ' ', 'w', ' ', ' '},
        {' ', 'w', 'w', 'w', ' ', 'w', 'w', 'w'},
        {' ', 'k', ' ', 'w', ' ', ' ', 'k', 'w'},
        {'w', 'w', ' ', 'w', 'n', 'w', 'w', 'w'},
        {'n', ' ', ' ', 'w', ' ', ' ', ' ', 'P'}
    };

char maze[8][8];
void setMaze(){
  // Reset maze
  for (int i=0; i<8; i++){
      for (int j=0; j<8; j++){
          maze[i][j] = startingMaze[i][j];
      }
  }

  // Place player
  playerOne_coord[0] = 7;
  playerOne_coord[1] = 7;
  playerOne.direction = 1;
  playerOne.scope_direction = playerOne.direction;
  playerOne.keys = 0;

  // Place enemies
  enemy_Left_coord[0] = 0;
  enemy_Left_coord[1] = 0;

  enemy_Middle_coord[0] = 3;
  enemy_Middle_coord[1] = 2;

  enemy_Right_coord[0] = 3;
  enemy_Right_coord[1] = 6;

  // Place keys
  key1_coord[0] = 5;
  key1_coord[1] = 6;

  key2_coord[0] = 0;
  key2_coord[1] = 3;

  key3_coord[0] = 5;
  key3_coord[1] = 1;

  exitLocks = 3;
}

void drawBox(unsigned char xStart, unsigned char xEnd, unsigned char yStart, unsigned char yEnd, unsigned char red, unsigned char blue, unsigned char green){
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
  
  unsigned char greenBlue = green << 5 | blue;        // Green bits 2-0 w/ Blue bits 4-0
  unsigned char redGreen = red << 3 | (green >> 3);   // Red bits 5-0 w/ Green bits 5-3
  Send_Command(0x2c);    // RAMWR: memory write 
  for (int i = 0; i < ((xEnd-xStart+1)*(yEnd-yStart+1)); i++){
    Send_Data(greenBlue);   
    Send_Data(redGreen);
  }
}

void drawMaze(){
  old_scopeX = scopeX;
  old_scopeY = scopeY;

  // for the entire screen 2-129x and 1-128y, find the player
  for(int i=MAZE_XSTART/16; i<MAZE_XEND/16; i++){
    for (int j=MAZE_YSTART/16; j<MAZE_YEND/16; j++){
      if(maze[j][i]=='P'){
        // Determine flashlight scope coordinates
        if(playerOne.scope_direction==1 && (i-1)>=0){
          scopeX = i-1;
          scopeY = j;
        }
        else if(playerOne.scope_direction==2 && (i+1)<=7){
          scopeX = i+1;
          scopeY = j;
        }
        else if(playerOne.scope_direction==3 && (j-1)>=0){
          scopeX = i;
          scopeY = j-1;
        }
        else if(playerOne.scope_direction==4 && (j+1)<=7){
          scopeX = i;
          scopeY = j+1;
        }

        // Clear old scope
        if ((old_scopeX != scopeX || old_scopeY != scopeY) && maze[old_scopeY][old_scopeX] != ' '){
          drawBox(MAZE_XSTART+(old_scopeX*16), MAZE_XSTART+(old_scopeX*16)+15, MAZE_YSTART+(old_scopeY*16), MAZE_YSTART+(old_scopeY*16)+15, 0x0, 0x0, 0x0);
        }

        // Draw object in flashlight scope
        if(scopeY == enemy_Left_coord[0] && scopeX == enemy_Left_coord[1]){
          if(flashlightPressed)
            enemy_Left.stunned = true;
          drawEnemy(MAZE_XSTART+((enemy_Left_coord[1])*16), MAZE_XSTART+((enemy_Left_coord[1])*16)+15, MAZE_YSTART+((enemy_Left_coord[0])*16), MAZE_YSTART+((enemy_Left_coord[0])*16)+15, enemy_Left);
        }
        else if(scopeY == enemy_Middle_coord[0] && scopeX == enemy_Middle_coord[1]){
          if(flashlightPressed)
            enemy_Middle.stunned = true;
          drawEnemy(MAZE_XSTART+((enemy_Middle_coord[1])*16), MAZE_XSTART+((enemy_Middle_coord[1])*16)+15, MAZE_YSTART+((enemy_Middle_coord[0])*16), MAZE_YSTART+((enemy_Middle_coord[0])*16)+15, enemy_Middle);
        } 
        else if(scopeY == enemy_Right_coord[0] && scopeX == enemy_Right_coord[1]){
          if(flashlightPressed)
            enemy_Right.stunned = true;
          drawEnemy(MAZE_XSTART+((enemy_Right_coord[1])*16), MAZE_XSTART+((enemy_Right_coord[1])*16)+15, MAZE_YSTART+((enemy_Right_coord[0])*16), MAZE_YSTART+((enemy_Right_coord[0])*16)+15, enemy_Right);
        } 
        else if(maze[scopeY][scopeX]=='k'){
          drawKey(MAZE_XSTART+(scopeX*16), MAZE_XSTART+(scopeX*16)+15, MAZE_YSTART+(scopeY*16), MAZE_YSTART+(scopeY*16)+15, key_sprite);
        }
        else if(maze[scopeY][scopeX]=='w'){
          if (playerOne.scope_direction==1 || playerOne.scope_direction==2)
            drawWall(MAZE_XSTART+(scopeX*16), MAZE_XSTART+(scopeX*16)+15, MAZE_YSTART+(scopeY*16), MAZE_YSTART+(scopeY*16)+15, wallY_sprite);
          else if (playerOne.scope_direction==3 || playerOne.scope_direction==4)
            drawWall(MAZE_XSTART+(scopeX*16), MAZE_XSTART+(scopeX*16)+15, MAZE_YSTART+(scopeY*16), MAZE_YSTART+(scopeY*16)+15, wallX_sprite);            
        }
        else if(scopeY == exit_coord[0] && scopeX == exit_coord[1]){
          drawExit(MAZE_XSTART+(scopeX*16), MAZE_XSTART+(scopeX*16)+15, MAZE_YSTART+(scopeY*16), MAZE_YSTART+(scopeY*16)+15, exit_sprite);
        }
        else if((maze[scopeY][scopeX]==' ' || maze[scopeY][scopeX]=='n') && ((scopeY!=enemy_Left_coord[0] || scopeY!=enemy_Middle_coord[0] || scopeY!=enemy_Right_coord[0]) && (scopeX!=enemy_Left_coord[1] || scopeX!=enemy_Middle_coord[1] || scopeX!=enemy_Right_coord[1]))){
          drawBox(MAZE_XSTART+(scopeX*16), MAZE_XSTART+(scopeX*16)+15, MAZE_YSTART+(scopeY*16), MAZE_YSTART+(scopeY*16)+15, 0xff, 0xff, 0xff);
        }

        // Draw player
        drawPlayer(MAZE_XSTART+(i*16), MAZE_XSTART+(i*16)+15, MAZE_YSTART+(j*16), MAZE_YSTART+(j*16)+15, playerOne);
        playerOne_coord[0] = j;
        playerOne_coord[1] = i;
      }
      else if((maze[j][i]==' ' || maze[j][i]=='n') && (j!=scopeY || i!=scopeX)){
        drawBox(MAZE_XSTART+(i*16), MAZE_XSTART+(i*16)+15, MAZE_YSTART+(j*16), MAZE_YSTART+(j*16)+15, 0x0, 0x0, 0x0);
      }
    }
  }
    // draw object from maze coordinate
}

void checkKeyPickup(int row, int col){
  if(maze[row][col] == 'k')
    playerOne.keys++;
}

void movePlayer(int direction){
  if(direction==1){
    // move left
    for(int i=0 ; i<8; i++){
      for (int j=0; j<8; j++){
        if(maze[j][i]=='P' && i>0 && maze[j][i-1] != 'w'){
          maze[j][i] = ' ';
          checkKeyPickup(j,i-1);
          maze[j][i-1] = 'P';
          return;
        }
      }
    }
  }
  else if(direction==2){
    // move right
    for(int i=0 ; i<8; i++){
      for (int j=0; j<8; j++){
        if(maze[j][i]=='P' && i<7 && maze[j][i+1] != 'w'){
          maze[j][i] = ' ';
          checkKeyPickup(j,i+1);
          maze[j][i+1] = 'P';
          return;
        }
      }
    }
  }
  else if(direction==3){
    // move up
    for(int i=0 ; i<8; i++){
      for (int j=0; j<8; j++){
        if(maze[j][i]=='P' && j>0 && maze[j-1][i] != 'w'){
          maze[j][i] = ' ';
          checkKeyPickup(j-1,i);
          maze[j-1][i] = 'P';
          return;
        }
      }
    }
  }
  else if(direction==4){
    // move down
    for(int i=0 ; i<8; i++){
      for (int j=0; j<8; j++){
        if(maze[j][i]=='P' && j<7 && maze[j+1][i] != 'w'){
          maze[j][i] = ' ';
          checkKeyPickup(j+1,i);
          maze[j+1][i] = 'P';
          return;
        }
      }
    }
  }
}
