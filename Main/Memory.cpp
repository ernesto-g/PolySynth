#include "24LC256.h"
#define EEPROM_ADDRESS 0x50 // 24LC256 EEPROM Address in i2c bus 

static E24LC256 mem(EEPROM_ADDRESS);  


void mem_init(void)
{
  
}

