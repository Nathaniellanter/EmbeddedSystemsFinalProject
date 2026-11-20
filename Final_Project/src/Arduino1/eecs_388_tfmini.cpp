/******************************************************************************
 *   Header Includes
 *******************************************************************************/
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

#include "eecs_388_lib.h"

void setup() 
{
gpio_mode(GPIO_13, GPIO_OUTPUT);
gpio_mode(GPIO_12, GPIO_OUTPUT);
gpio_mode(GPIO_11, GPIO_OUTPUT);   /* ADDED (Task 1.5, blue LED) */

uart_init();

return;
}

void loop() 
{
uint16_t dist = 0;              /* LIDAR distance data is 16 bits. */
uint8_t frame_count =0;
ser_printline("Setup completed.");
ser_write( '\n' );

while( 1 )
    { 
    if ('Y' == ser_read() && 'Y' == ser_read()) 
        {
            uint8_t dist_l = ser_read();   /* byte 3 */
        uint8_t dist_h = ser_read();   /* byte 4 */
        dist = ((uint16_t)dist_h << 8) | dist_l;
        uint8_t sum = 0x59 + 0x59 + dist_l + dist_h;   /* ADDED (Task 1.4) */

        /* CHANGED (Task 1.4): was a loop that read and discarded 5 bytes */
        for (uint8_t i = 0; i < 4; i++)      /* bytes 5-8 */
            {
            sum += ser_read();
            }
        uint8_t chk_byte = ser_read();       /* byte 9 */
        bool chk_ok = (sum == chk_byte);     /* uint8_t wraps = low byte */

        /* Task 1.3: 75 cm threshold per the handout */
        if (dist < 75) 
            {
            gpio_write(GPIO_13, ON);    /* red */
            gpio_write(GPIO_12, OFF);   /* green */
            }
        else 
            {
            gpio_write(GPIO_13, OFF);
            gpio_write(GPIO_12, ON);
            }

        /* ADDED (Task 1.5): out-of-range -> all LEDs off */
        if (dist < 30 || dist >= 1200) 
            {
            gpio_write(GPIO_13, OFF);
            gpio_write(GPIO_12, OFF);
            gpio_write(GPIO_11, OFF);   /* change OFF to ON for blue-only */
            }

        /* print about once per second (100 frames/sec) */
        if (++frame_count >= 100) 
            {
            frame_count = 0;
            ser_printf("dist = %u cm", dist);
            ser_printf("checksum %s", chk_ok ? "OK" : "BAD");   /* ADDED */
            }

        }
    }

return;
}