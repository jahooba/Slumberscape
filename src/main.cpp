/*        Your Name & E-mail: Joshua Pena jpena079@ucr.edu

*          Discussion Section:23

 *         Assignment: Custom Final Project - Slumberscape Demo #3

 *         Description:  Implemented the logic for flashlight stunning enemies - 3rd complexity
                         

 *        

 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *

 *         Demo Link: https://youtu.be/QgebrILdqKc

 */
#include "timerISR.h"
#include "helper.h"
#include "i2c.h"
#include "periph.h"
#include "spiAVR.h"
#include "st7735Helper.h"
#include "../include/Assets/maze.h"


bool systemOn = false;    // determines screen on/off
unsigned char startButtonTick = 0;    // determines which start button press length option
bool gameStarted = false;   // determines is game menu screen is on/off
bool died = false;
bool escaped = false;


bool flashlightCharged = true;    // determines if flashlight can be used
unsigned char cooldownTick = 0;   // determines when flashlight is ready to use
unsigned char rgbTick = 0;        // determines flash lengths for RGB LED

int rgbLED_States[4] = {0x00, 0x20, 0x40, 0x70};
unsigned char rgbState = rgbLED_States[0];    // data for RGB LED -starts in off state

int keyLED_States[4] = {0x00, 0x01, 0x03, 0x07};
unsigned char keyState = keyLED_States[0];    // data for yellow LEDs -starts in off state

int xAxis = 0;    // "x-axis" for joystick
int yAxis = 0;    // "y-axis" for joystick


unsigned char enemy_Left_tick = 0;
unsigned char enemy_Middle_tick = 0;
unsigned char enemy_Right_tick = 0;


#define NUM_TASKS 12

//Task struct for concurrent synchSMs implementations
typedef struct _task{
  signed char state;          //Task's current state
  unsigned long period;       //Task period
  unsigned long elapsedTime;  //Time elapsed since last task tick
  int (*TickFct)(int);        //Task tick function
} task;

//Periods for each task
const unsigned long StartButton_PERIOD = 50;
const unsigned long MoveJoystick_PERIOD = 40;
const unsigned long Screen_PERIOD = 50;
const unsigned long FlashlightButton_PERIOD = 50;
const unsigned long RGBLED_PERIOD = 60;
const unsigned long KeysLEDs_PERIOD = 60;
const unsigned long OutputLEDs_PERIOD = 70;
const unsigned long JoystickButton_PERIOD = 50;
const unsigned long Enemy_Left_Movement_PERIOD = 90;
const unsigned long Enemy_Middle_Movement_PERIOD = 90;
const unsigned long Enemy_Right_Movement_PERIOD = 90;
const unsigned long Game_Check_PERIOD = 1;
const unsigned long GCD_PERIOD = 1;    //GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//Declare your task states
enum StartButton {StartButton_START, StartButton_released, StartButton_pressed, StartButton_shutDown};
enum FlashlightButton {FlashlightButton_START, FlashlightButton_released, FlashlightButton_pressed, FlashlightButton_cooldown};
enum RGBLED {RGBLED_START, RGBLED_wait, RGBLED_flashlight, RGBLED_died, RGBLED_escaped};
enum KeysLEDs {KeysLEDs_START, KeysLEDs_wait};
enum OutputLEDs {OutputLEDs_START, OutputLEDs_output};
enum JoystickButton {JoystickButton_START, JoystickButton_released, JoystickButton_pressed};
enum MoveJoystick {MoveJoystick_START, MoveJoystick_wait, MoveJoystick_read};
enum Screen {Screen_START, Screen_off, Screen_menu, Screen_game, Screen_died, Screen_escaped};
enum Enemy_Left_Movement {Enemy_Left_Movement_START, Enemy_Left_Movement_wait, Enemy_Left_Movement_move, Enemy_Left_Movement_stunned};
enum Enemy_Middle_Movement {Enemy_Middle_Movement_START, Enemy_Middle_Movement_wait, Enemy_Middle_Movement_move, Enemy_Middle_Movement_stunned};
enum Enemy_Right_Movement {Enemy_Right_Movement_START, Enemy_Right_Movement_wait, Enemy_Right_Movement_move, Enemy_Right_Movement_stunned};
enum Game_Check {Game_Check_START, Game_Check_wait, Game_Check_gaming};

// Scheduler
void TimerISR() {
  for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {        // Iterate through each task in the task array
    if ( tasks[i].elapsedTime == tasks[i].period ) {        // Check if the task is ready to tick
      tasks[i].state = tasks[i].TickFct(tasks[i].state);      // Tick and set the next state for this task
      tasks[i].elapsedTime = 0;                               // Reset the elapsed time for the next tick
    }
    tasks[i].elapsedTime += GCD_PERIOD;                     // Increment the elapsed time by GCD_PERIOD
  }
}

// Shift register function
void shiftOut(unsigned char data) {
    for (int i = 7; i >= 0; i--) {
      // Write each bit to SER: portd = clear PD6 then write data into PD6
      PORTD = (PORTD & ~(1 << 6)) | (((data >> i) & 0x01) << 6);
      
      // Pulse the SRCLK
      PORTD |= (1 << 7);  // Set SRCLK PD7
      PORTD &= ~(1 << 7); // Clear SRCLK PD7
    }
    // Pulse the RCLK to update output
    PORTD &= ~(1 << 4); // Clear RCLK PD4
    PORTD |= (1 << 4);  // Set RCLK PD4
}

//Tick functions for each task
int TickFct_StartButton(int state){
  switch (state){   // Transitions
    case StartButton_START:
      systemOn = false;
      gameStarted = false;
      startButtonTick = 0;
      state = StartButton_released;
      break;
    
    case StartButton_released:
      if(GetBit(PINC, 3))
        state = StartButton_pressed;
      else
        state = StartButton_released;
      break;

    case StartButton_pressed:
      if(!(GetBit(PINC, 3))){
        // if system off and start button released, then turn on system and return to release state
        if(!systemOn){
          systemOn = true;
        }
        // else if system on and game hasn't/has started and start button released at <2s, then start/reset game and return to release state
        else{
          gameStarted = (!gameStarted && startButtonTick < 40);
        }

        startButtonTick = 0;
        state = StartButton_released;
      }
      else{
        startButtonTick++;
        // else if system on and start button pressed for >=2s, then turn off system and return to release state
        if(systemOn && startButtonTick>=40){
          startButtonTick = 0;
          systemOn = false;
          gameStarted = false;
          state = StartButton_shutDown;
        }
        else
          state = StartButton_pressed;
      }
      break;

    case StartButton_shutDown:
      if(!(GetBit(PINC, 3))){
        state = StartButton_released;
      }
      else
        state = StartButton_shutDown;
      break;

    default:
      break;
  }
  switch (state){   // Actions
    case StartButton_START:
      break;

    default:
      break;
  }
  return state;
}

int TickFct_FlashlightButton(int state){
  switch (state){
    case FlashlightButton_START:
      flashlightPressed = false;
      flashlightCharged = true;
      cooldownTick = 0;
      state = FlashlightButton_released;
      break;
    
    case FlashlightButton_released:
      // if game started and button pressed and flashlight is "charged", flashlight is pressed and go to pressed state
      if(gameStarted && flashlightCharged && GetBit(PINC, 4)){
        flashlightPressed = true;
        state = FlashlightButton_pressed;
      }
      // else stay in released state
      else
        state = FlashlightButton_released;
      break;
    
    case FlashlightButton_pressed:
      // if button is released, then start flashlight cooldown
      if(!(GetBit(PINC, 4))){
        flashlightPressed = false;
        state = FlashlightButton_cooldown;
      }
      // else stay in pressed state
      else
        state = FlashlightButton_pressed;
      break;

    case FlashlightButton_cooldown:
      if(systemOn && gameStarted){
        if(cooldownTick<200){    // 10s cooldown
          cooldownTick++;
          flashlightCharged = false;
          state = FlashlightButton_cooldown;
        }
        else{
          flashlightCharged = true;
          cooldownTick = 0;
          state = FlashlightButton_released;
        }
      }
      else{
        cooldownTick = 0;
        flashlightCharged = true;
        state = FlashlightButton_released;
      }
      break;
    
    default:
      break;
  }
  return state;
}

int TickFct_RGBLED(int state){
  switch (state){     // Transitions
    case RGBLED_START:
      rgbTick = 0;
      // set RGB LED to off by shift register
      state = RGBLED_wait;
      break;
    
    case RGBLED_wait:
      if(systemOn && gameStarted){
        // set RGB off by shift register
        rgbState = rgbLED_States[0];
        // if enemy reaches player tile, then go to died state
        if(died){
          state = RGBLED_died;
        }
        // else if player flashlight is pressed and flashlight is charged, then go to flashlight state
        else if(flashlightPressed && flashlightCharged){
          state = RGBLED_flashlight;
        }
        // else if player exits the door and wins, then go to escaped state
        else if(escaped){
          state = RGBLED_escaped;
        }
        // else stay and do nothing in wait state
        else
          state = RGBLED_wait; 
      }
      else{
        rgbState = rgbLED_States[0];
        state = RGBLED_wait;
      }
      break;

    case RGBLED_flashlight:
      // white RGB LED on by shift register
      rgbState = rgbLED_States[3];
      // if system is on or user hasn't reset game, then turn on white RGB LED for 1.2s by shift register
      if(systemOn && gameStarted && rgbTick<20){
        state = RGBLED_flashlight;
      }
      // return to wait state
      else{
        rgbState = rgbLED_States[0];
        flashlightPressed = false;
        rgbTick = 0;
        state = RGBLED_wait;
      }
      break;

    case RGBLED_died:
      rgbState = rgbLED_States[2];
      // if system is on or user hasn't reset game, then turn on red RGB LED by shift register
      // else return to wait 
      state = (systemOn && gameStarted) ? RGBLED_died : RGBLED_wait;
      break;

    case RGBLED_escaped:
      // if system is on or user hasn't reset game, then flash green RGB LED quickly for 3s  by shift register
      if(systemOn && gameStarted && rgbTick<50){
        state = RGBLED_escaped;
      }
      // else return to wait 
      else{
        rgbTick = 0;
        state = RGBLED_wait;
      }
      break;
    
    default:
      break;
  }
  switch (state){     // Actions
    case RGBLED_START:
      break;

    case RGBLED_flashlight:
      rgbTick++;
      break;

    case RGBLED_escaped:
      rgbTick++;
      rgbState = (rgbTick%2) ? rgbLED_States[1] : rgbLED_States[0]; 
      break;
    
    default:
      break;
  }
  return state;
}

int TickFct_KeysLEDs(int state){
  switch (state){
    case KeysLEDs_START:
      state = KeysLEDs_wait;
      break;
    
    case KeysLEDs_wait:
      if(systemOn && gameStarted){
        // if player picks up key, then increment keys 
        // else if player uses a key, then decrement keys 
        // wait in this state
        keyState = keyLED_States[playerOne.keys];
      }
      else{
        keyState = keyLED_States[0];
      }
      state = KeysLEDs_wait;
      break;

    default:
      break;
  }
  return state;
}

int TickFct_OutputLEDs(int state){
  switch (state){
    case OutputLEDs_START:
      state = OutputLEDs_output;
      break;
    
    case OutputLEDs_output:
      shiftOut(rgbState | keyState);
      state = OutputLEDs_output;
      break;
    
    default:
      break;
  }
  return state;
}

int TickFct_JoystickButton(int state){
  switch (state){
    case JoystickButton_START:
      state = JoystickButton_released;
      break;
    
    case JoystickButton_released:
      if(!(GetBit(PINC, 0)))
        state = JoystickButton_pressed;
      else
        state = JoystickButton_released;
      break;

    case JoystickButton_pressed:
      if(GetBit(PINC, 0)){
        // if player is on exit door, then use available key
        if(systemOn && gameStarted && (playerOne_coord[0]==exit_coord[0] && playerOne_coord[1]==exit_coord[1]) && playerOne.keys>0){
          playerOne.keys--; 
          exitLocks--;
        }
        state = JoystickButton_released;
      }
      // else if player is on secret, then pick up and show note, returning to released state
      else // else do nothing and return to released state
        state = JoystickButton_pressed;
      break;

    default:
      break;
  }
  switch (state){
    case JoystickButton_START:
      break;
    
    default:
      break;
  }
  return state;
}

int TickFct_MoveJoystick(int state){
  switch (state){
    case MoveJoystick_START:
      state = MoveJoystick_wait;
      break;

    case MoveJoystick_wait:
      playerOne.direction = 0;
      state = (gameStarted)? MoveJoystick_read : MoveJoystick_wait;
      break;
    
    case MoveJoystick_read:
      xAxis = map(ADC_read(1), 0, 1023, 0, 7);
      yAxis = map(ADC_read(2), 0, 1023, 0, 7);

      playerOne.scope_direction = (playerOne.direction!=0) ? playerOne.direction : playerOne.scope_direction;

      // Centered Joystick
      if(xAxis==3 && yAxis==3){
        movePlayer(playerOne.direction);
        playerOne.direction = 0;
      }

      // move player in appropriate x-direction
      if(xAxis!=3 && yAxis==3){
        if(xAxis==0)
          playerOne.direction = 1;
        else if(xAxis>=1 && xAxis <=2)
          playerOne.scope_direction = 1;
        else if(xAxis==6)
          playerOne.direction = 2;
        else if(xAxis>=4 && xAxis<=5)
          playerOne.scope_direction = 2;
      }
      // move player in appropriate y-direction
      else if(xAxis==3 && yAxis!=3){
        if(yAxis==0)
          playerOne.direction = 4;
        else if(yAxis>=1 && yAxis <=2)
          playerOne.scope_direction = 4;
        else if(yAxis==6)
          playerOne.direction = 3; 
        else if(yAxis>=4 && yAxis<=5)
          playerOne.scope_direction = 3;
      }
      
      state = (gameStarted)? MoveJoystick_read : MoveJoystick_wait;
      break;

    default:
      break;
  }
  return state;
}

int TickFct_Screen(int state){
  switch (state){
  case Screen_START:
    state = Screen_off;
    break;

  case Screen_off:
    drawBox(0, 130, 0, 130, 0xff, 0xff, 0xff);
    state = (systemOn)? Screen_menu : Screen_off;
    break;

  case Screen_menu:
    drawBox(0, 130, 0, 130, 0x00, 0x00, 0x00);
    // Draw title screen
    if(gameStarted){
      setMaze();
      state = Screen_game;
    }
    else if(!systemOn){
      state = Screen_off;
    }
    else{
      state = Screen_menu;
    }
    break;

  case Screen_game:
    drawMaze();
    if(gameStarted && !died && !escaped){
      state = Screen_game;
    }
    else if (died){
      state = Screen_died;
    }
    else if (escaped){
      state = Screen_escaped;
    }
    else if(!gameStarted || !systemOn){
      state = Screen_off;
    }
    break;

  case Screen_died:
    drawBox(0, 130, 0, 130, 0xff, 0x00, 0x00);
    state = (gameStarted) ? Screen_died : Screen_off;
    break;
  
  case Screen_escaped:
    drawBox(0, 130, 0, 130, 0x0f, 0x0f, 0x3f);
    state = (gameStarted) ? Screen_escaped : Screen_off;
    break;

  default:
    break;
  }
  return state;
}

int TickFct_Enemy_Left_Movement(int state){
  switch (state){
    case Enemy_Left_Movement_START:
      state = Enemy_Left_Movement_wait;
      break;

    case Enemy_Left_Movement_wait:
        enemy_Left.stunned = false;
        enemy_Left_tick = 0;
        enemy_Left_moves = 0;
        state = (gameStarted)? Enemy_Left_Movement_move : Enemy_Left_Movement_wait;
      break;

    case Enemy_Left_Movement_move:
      if(enemy_Left_tick==12){    // Every 1s
        enemy_Left_tick = 0;
        if(enemy_Left_moves <= 4){
          if(!enemy_Left.forward && enemy_Left_moves < 0){
            enemy_Left.forward = !enemy_Left.forward;
            enemy_Left_moves = 0; 
          }
          if(enemy_Left.forward){
            enemy_Left_coord[0]++;
            enemy_Left_moves++;
          }
          else{
            enemy_Left_coord[0]--;
            enemy_Left_moves--;
          }
        }
        else if(enemy_Left_moves >=5 && enemy_Left_moves <= 6){
          if(enemy_Left.forward){
            enemy_Left_coord[1]++;
            enemy_Left_moves++;
          }
          else{
            enemy_Left_coord[1]--;
            enemy_Left_moves--;
          }
        }
        else if(enemy_Left_moves <= 7){
          if(enemy_Left.forward){
            enemy_Left_coord[0]++;
            enemy_Left_moves++;
          }
          else{
            enemy_Left_coord[0]--;
            enemy_Left_moves--;
          }
        }
        else if(enemy_Left_moves==8){
          enemy_Left.forward = !enemy_Left.forward;
          enemy_Left_moves--;
        }
      }

      if (gameStarted && !enemy_Left.stunned){
        state = Enemy_Left_Movement_move;
      }
      else if (gameStarted && enemy_Left.stunned){
        enemy_Left_tick = 0;
        state = Enemy_Left_Movement_stunned;
      }
      else if (!gameStarted){
        state = Enemy_Left_Movement_wait;
      }
      break;

    case Enemy_Left_Movement_stunned:
      if(gameStarted && enemy_Left_tick<=55){    // Stunned for 5s
        state = Enemy_Left_Movement_stunned;
      }
      else if(gameStarted && enemy_Left_tick>55){
        enemy_Left_tick = 0;
        enemy_Left.stunned = false; 
        state = Enemy_Left_Movement_move;
      }
      else if(!gameStarted){
        state = Enemy_Left_Movement_wait;
      }
      break;
    
    default:
      break;
  }
  switch (state){
    case Enemy_Left_Movement_START:
      break;

    case Enemy_Left_Movement_move:
      enemy_Left_tick++;
      break;
    
    case Enemy_Left_Movement_stunned:
      enemy_Left_tick++;
      break;

    default:
      break;
  }
  return state;
}

int TickFct_Enemy_Middle_Movement(int state){
  switch (state){
    case Enemy_Middle_Movement_START:
      state = Enemy_Middle_Movement_wait;
      break;
    
    case Enemy_Middle_Movement_wait:
        enemy_Middle.stunned = false;
        enemy_Middle_tick = 0;
        enemy_Middle_moves = 0;
        state = (gameStarted)? Enemy_Middle_Movement_move : Enemy_Middle_Movement_wait;
      break;

    case Enemy_Middle_Movement_move:
      if(enemy_Middle_tick==12){    // Every 1s
        enemy_Middle_tick = 0;
        if(enemy_Middle_moves <= 2){
          if(!enemy_Middle.forward && enemy_Middle_moves < 0){
            enemy_Middle.forward = !enemy_Middle.forward; 
            enemy_Middle_moves = 0;
          }
          if(enemy_Middle.forward){
            enemy_Middle_coord[0]--;
            enemy_Middle_moves++;
          }
          else{
            enemy_Middle_coord[0]++;
            enemy_Middle_moves--;
          }
        }
        else if(enemy_Middle_moves >=3 && enemy_Middle_moves < 6){
          if(enemy_Middle.forward){
            enemy_Middle_coord[1]++;
            enemy_Middle_moves++;
          }
          else{
            enemy_Middle_coord[1]--;
            enemy_Middle_moves--;
          }
        }
        else if(enemy_Middle_moves==6){
          enemy_Middle.forward = !enemy_Middle.forward;
          enemy_Middle_moves--;
        }
      }

      if (gameStarted && !enemy_Middle.stunned){
        state = Enemy_Middle_Movement_move;
      }
      else if (gameStarted && enemy_Middle.stunned){
        enemy_Middle_tick = 0;
        state = Enemy_Middle_Movement_stunned;
      }
      else if (!gameStarted){
        state = Enemy_Middle_Movement_wait;
      }
      break;

    case Enemy_Middle_Movement_stunned:
      if(gameStarted && enemy_Middle_tick<=55){    // Stunned for 5s
        state = Enemy_Middle_Movement_stunned;
      }
      else if(gameStarted && enemy_Middle_tick>55){
        enemy_Middle_tick = 0;
        enemy_Middle.stunned = false; 
        state = Enemy_Middle_Movement_move;
      }
      else if(!gameStarted){
        state = Enemy_Middle_Movement_wait;
      }
      break;
    
    default:
      break;
  }
  switch (state){
    case Enemy_Middle_Movement_START:
      break;

    case Enemy_Middle_Movement_move:
      enemy_Middle_tick++;
      break;

    case Enemy_Middle_Movement_stunned:
      enemy_Middle_tick++;
      break;
    
    default:
      break;
  }
  return state;
}

int TickFct_Enemy_Right_Movement(int state){
  switch (state){
    case Enemy_Right_Movement_START:
      state = Enemy_Right_Movement_wait;
      break;

    case Enemy_Right_Movement_wait:
        enemy_Right.stunned = false;
        enemy_Right_tick = 0;
        enemy_Right_moves = 0;
        state = (gameStarted)? Enemy_Right_Movement_move : Enemy_Right_Movement_wait;
      break;
    
    case Enemy_Right_Movement_move:
      if(enemy_Right_tick==12){    // Every 1s
        enemy_Right_tick = 0;
        if(enemy_Right_moves < 1){
          if(!enemy_Right.forward && enemy_Right_moves < 0){
            enemy_Right.forward = !enemy_Right.forward;
            enemy_Right_moves = 0; 
          }
          if(enemy_Right.forward){
            enemy_Right_coord[1]++;
            enemy_Right_moves++;
          }
          else{
            enemy_Right_coord[1]--;
            enemy_Right_moves--;
          }
        }
        else if(enemy_Right_moves >=1 && enemy_Right_moves < 3){
          if(enemy_Right.forward){
            enemy_Right_coord[0]--;
            enemy_Right_moves++;
          }
          else{
            enemy_Right_coord[0]++;
            enemy_Right_moves--;
          }
        }
        else if(enemy_Right_moves >=3 && enemy_Right_moves<6){
          if(enemy_Right.forward){
            enemy_Right_coord[1]--;
            enemy_Right_moves++;
          }
          else{
            enemy_Right_coord[1]++;
            enemy_Right_moves--;
          }
        }
        else if(enemy_Right_moves >=6 && enemy_Right_moves<10){
          if(enemy_Right.forward){
            enemy_Right_coord[0]++;
            enemy_Right_moves++;
          }
          else{
            enemy_Right_coord[0]--;
            enemy_Right_moves--;
          }
        }
        else if(enemy_Right_moves==10){
          if(enemy_Right.forward){
            enemy_Right_coord[1]++;
            enemy_Right_moves++;
          }
          else{
            enemy_Right_coord[1]--;
            enemy_Right_moves--;
          }
        }
        else if(enemy_Right_moves==11){
          enemy_Right.forward = !enemy_Right.forward;
          enemy_Right_moves--;
        }
      }
      
      if (gameStarted && !enemy_Right.stunned){
        state = Enemy_Right_Movement_move;
      }
      else if (gameStarted && enemy_Right.stunned){
        enemy_Right_tick = 0;
        state = Enemy_Right_Movement_stunned;
      }
      else if (!gameStarted){
        state = Enemy_Right_Movement_wait;
      }
      break;

    case Enemy_Right_Movement_stunned:
      if(gameStarted && enemy_Right_tick<=55){    // Stunned for 5s
        state = Enemy_Right_Movement_stunned;
      }
      else if(gameStarted && enemy_Right_tick>55){
        enemy_Right_tick = 0;
        enemy_Right.stunned = false; 
        state = Enemy_Right_Movement_move;
      }
      else if(!gameStarted){
        state = Enemy_Right_Movement_wait;
      }
      break;
    
    default:
      break;
  }
  switch (state){
    case Enemy_Right_Movement_START:
      break;
    
    case Enemy_Right_Movement_move:
      enemy_Right_tick++;
      break;

    case Enemy_Right_Movement_stunned:
      enemy_Right_tick++;
      break;
    
    default:
      break;
  }
  return state;
}

int TickFct_Game_Check(int state){
  switch (state){
    case Game_Check_START:
      state = Game_Check_wait;
      break;

    case Game_Check_wait:
      died = false;
      escaped = false;
      state = (gameStarted) ? Game_Check_gaming : Game_Check_wait; 
      break;

    case Game_Check_gaming:
      if ((playerOne_coord[0]==enemy_Left_coord[0] && playerOne_coord[1]==enemy_Left_coord[1]) 
          || (playerOne_coord[0]==enemy_Middle_coord[0] && playerOne_coord[1]==enemy_Middle_coord[1])
          || (playerOne_coord[0]==enemy_Right_coord[0] && playerOne_coord[1]==enemy_Right_coord[1])){
        died = true; 
      }
      else if (exitLocks==0){
        escaped = true;
      }
      state = (gameStarted) ? Game_Check_gaming : Game_Check_wait;
      break;
    
    default:
      break;
  }

  return state;
}

int main(void) {
  //Input Ports 0/pins 1 and Outputs Ports 1/pins 0
  DDRB = 0xff;    // outputs (PB5: LCD SCK, PB3: LCD SDA for MOSI pin, PB2: LCD CS for SS pin, PB1: LCD AO (send data or command), PB0: LCD RESET)
  PORTB= 0x01;    // Remember PB0: Reset active low

  DDRC = 0x00;    // inputs (PC4: flashlight button, PC3: start button, PC2: VRX joystick, PC1: VRY joystick, PC0: SW joystick)
  PORTC= 0xff;

  DDRD = 0xf0;    // outputs (PD7: shift register shift clock (SRCLK), PD6: shift register serial data in (SER), PD5: buzzer, PD4: shift register latch clock (RCLK))
  PORTD =0x10;    // Remember PD4: RCLK is active low 
                  // Q7: blue RGB LED, Q6: green RGB LED, Q5: red RGB LED, Q4: 1st yellow LED, Q3: 2nd yellow LED, Q2: 3rd yellow LED
  
  ADC_init();
  SPI_INIT();
  ST7735_init();

  //Initialize tasks
  tasks[0].period = StartButton_PERIOD;
  tasks[0].state = StartButton_START;
  tasks[0].elapsedTime = tasks[0].period;
  tasks[0].TickFct = &TickFct_StartButton;

  tasks[1].period = FlashlightButton_PERIOD;
  tasks[1].state = FlashlightButton_START;
  tasks[1].elapsedTime = tasks[1].period;
  tasks[1].TickFct = &TickFct_FlashlightButton;

  tasks[2].period = RGBLED_PERIOD;
  tasks[2].state = RGBLED_START;
  tasks[2].elapsedTime = tasks[2].period;
  tasks[2].TickFct = &TickFct_RGBLED;

  tasks[3].period = JoystickButton_PERIOD;
  tasks[3].state = JoystickButton_START;
  tasks[3].elapsedTime = tasks[3].period;
  tasks[3].TickFct = &TickFct_JoystickButton;

  tasks[4].period = MoveJoystick_PERIOD;
  tasks[4].state = MoveJoystick_START;
  tasks[4].elapsedTime = tasks[4].period;
  tasks[4].TickFct = &TickFct_MoveJoystick;

  tasks[5].period = Screen_PERIOD;
  tasks[5].state = Screen_START;
  tasks[5].elapsedTime = tasks[5].period;
  tasks[5].TickFct = &TickFct_Screen;

  tasks[6].period = KeysLEDs_PERIOD;
  tasks[6].state = KeysLEDs_START;
  tasks[6].elapsedTime = tasks[6].period;
  tasks[6].TickFct = &TickFct_KeysLEDs;

  tasks[7].period = OutputLEDs_PERIOD;
  tasks[7].state = OutputLEDs_START;
  tasks[7].elapsedTime = tasks[7].period;
  tasks[7].TickFct = &TickFct_OutputLEDs;

  tasks[8].period = Enemy_Left_Movement_PERIOD;
  tasks[8].state = Enemy_Left_Movement_START;
  tasks[8].elapsedTime = tasks[8].period;
  tasks[8].TickFct = &TickFct_Enemy_Left_Movement;

  tasks[9].period = Enemy_Middle_Movement_PERIOD;
  tasks[9].state = Enemy_Middle_Movement_START;
  tasks[9].elapsedTime = tasks[9].period;
  tasks[9].TickFct = &TickFct_Enemy_Middle_Movement;

  tasks[10].period = Enemy_Right_Movement_PERIOD;
  tasks[10].state = Enemy_Right_Movement_START;
  tasks[10].elapsedTime = tasks[10].period;
  tasks[10].TickFct = &TickFct_Enemy_Right_Movement;

  tasks[11].period = Game_Check_PERIOD;
  tasks[11].state = Game_Check_START;
  tasks[11].elapsedTime = tasks[11].period;
  tasks[11].TickFct = &TickFct_Game_Check;
  
  TimerSet(GCD_PERIOD);
  TimerOn();
  while (1) {}
  return 0;
}