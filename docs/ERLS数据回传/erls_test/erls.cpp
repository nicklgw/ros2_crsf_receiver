
// 
// g++ -o erls erls.cpp crc8.cpp
// 
// ~/erls_test$ g++ -o erls erls.cpp crc8.cpp
// ~/erls_test$ ./erls
// C8 0A 08 00 4A 00 0A 00 27 10 46 DE 
// 回传数据 电压7.4V 电流1.0A 容量10000mAh 剩余70%
//

#include "crc8.h"
#include <stdio.h>
#include <string.h>

#define CRSF_MAX_PACKET_SIZE 64 // max declared len is 62+DEST+LEN on top of that = 64

#define CRSF_SYNC_BYTE 0XC8

#define CRSF_FRAMETYPE_BATTERY_SENSOR 0x08

#define PACKED __attribute__((packed))

typedef struct crsf_sensor_battery_s
{
    uint32_t voltage : 16;  // V * 10 big endian
    uint32_t current : 16;  // A * 10 big endian
    uint32_t capacity : 24; // mah big endian
    uint32_t remaining : 8; // %
} PACKED crsf_sensor_battery_t;


static inline uint16_t bswap16(uint16_t a)
{
  return (a<<8)|(a>>8);
}

void queuePacket(uint8_t addr, uint8_t type, const void *payload, uint8_t len);

Crc8 _crc(0xd5);

static void checkVbatt()
{
  uint32_t voltage = 74; // 7.4V // V * 10 big endian
  uint32_t current = 10;  // 1.0A // A * 10 big endian
  uint32_t capacity = 10000; // 10000mAh // mah big endian
  uint32_t remaining = 70; // 70% // %

	crsf_sensor_battery_t crsfbatt = { 0 };
	crsfbatt.voltage = bswap16(voltage);
	crsfbatt.current = bswap16(current);
	crsfbatt.capacity = ((capacity&0x000000FF)<<16) | (capacity&0x0000FF00) | ((capacity&0x00FF0000)>>16);
  crsfbatt.remaining = remaining;

	queuePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_BATTERY_SENSOR, &crsfbatt, sizeof(crsfbatt));
}

void queuePacket(uint8_t addr, uint8_t type, const void *payload, uint8_t len)
{
    uint8_t buf[CRSF_MAX_PACKET_SIZE];
    buf[0] = addr;
    buf[1] = len + 2; // type + payload + crc
    buf[2] = type;
    memcpy(&buf[3], payload, len);
    buf[len+3] = _crc.calc(&buf[2], len + 1);
	
	for(int i = 0; i < len+4; i++)
	{
		printf("%02X ", buf[i]);
	}
  printf("\n");
}

int main(int argc, char *argv[])
{
	checkVbatt();
	
	return 0;
}
