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

        /*FINAL PROJECT: greater than 200 cm is green, 
        100-200 is yellow, 60-100 is red brake hard, less than 60 is flashing red */ /*still need to add signals in logic so*/
        /*green*/
        static uint8_t flash_state = OFF;
        static unsigned long last_flash_time = 0;
         if (dist > 200 ) 
            {
            gpio_write(GPIO_13, OFF);   /* red */
            gpio_write(GPIO_12, ON);   /* green */
            gpio_write(GPIO_11, OFF);   /* Blue */
            }
            /*yellow*/

            
        else if (dist <= 200 && dist > 100) 
            {
            gpio_write(GPIO_13, ON);    /* red */
            gpio_write(GPIO_12, ON);   /* green */
            gpio_write(GPIO_11, OFF); /*blue*/
            }
        
        /*red*/
        else if (dist <= 100 && dist > 60) 
            {
            gpio_write(GPIO_13, ON);    /* red */
            
            gpio_write(GPIO_12, OFF);   /* green */
            gpio_write(GPIO_11, OFF);    /* blue */
            }
        /*flashing red*/
        else if (dist <= 60) 
            {
            unsigned long now = millis();
            if (now - last_flash_time >= 100UL)
                {
                flash_state = !flash_state;
                last_flash_time = now;
                }
            gpio_write(GPIO_13, flash_state);    /* red */
            
            gpio_write(GPIO_12, OFF);   /* green */
            gpio_write(GPIO_11, OFF);    /* blue */
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