#include <avr/io.h>
#define F_CPU 16000000UL
#define SCL_CLK 100000L
#define TOF10120_ADDR 0x52 //default address of i2c device
void i2c_init(void){
    TWSR = 0x00;
    TWBR = ((F_CPU/SCL_CLK) - 16) / 2;
    TWCR = (1<<TWEN);
}
void i2c_start(void){
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
}
void i2c_stop(void){
    TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}
void i2c_write(uint8_t data){
    TWDR = data;
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
}
uint8_t i2c_read_ack(void){
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    while(!(TWCR & (1<<TWINT)));
    return TWDR;
}
uint8_t i2c_read_nack(void){
    TWCR = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
    return TWDR;
}
