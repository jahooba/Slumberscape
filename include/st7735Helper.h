#include "helper.h"
#include "spiAVR.h"
#ifndef ST7735HELPER_H
#define ST7735HELPER_H

void HardwareReset(){
  PORTB = SetBit(PORTB, 0, 0);    //active
  _delay_ms(200);
  PORTB = SetBit(PORTB, 0, 1);    //inactive
  _delay_ms(200);
}

void Send_Command(unsigned char data){
  // cs low
  PORTB = SetBit(PORTB, 2, 0);
  // a0 to command
  PORTB = SetBit(PORTB, 1, 0);
  // spi send data
  SPI_SEND(data);
  // cs high
  PORTB = SetBit(PORTB, 2, 1);
}

void Send_Data(unsigned char data){
  // cs low
  PORTB = SetBit(PORTB, 2, 0);
  // a0 to data
  PORTB = SetBit(PORTB, 1, 1);
  // spi send data
  SPI_SEND(data);
  // cs high
  PORTB = SetBit(PORTB, 2, 1);
}

void ST7735_init(){
  HardwareReset();
  Send_Command(0x01);   // SWRESET: software reset
  _delay_ms(150);
  Send_Command(0x11);   // SLPOUT: sleep out & booster on
  _delay_ms(200);
  Send_Command(0x3a);   // COLMOD: interface pixel format
  Send_Data(0x05);      //for 16 bit color mode. You can pick any color mode you want
  _delay_ms(10);
  Send_Command(0x29);   // DISPON: display on
  _delay_ms(200);
}


#endif /* ST7735HELPER_H */