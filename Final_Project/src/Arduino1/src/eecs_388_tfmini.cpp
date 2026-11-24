/******************************************************************************
 *   Header Includes
 *******************************************************************************/
#include <Arduino.h>
#include <Servo.h>
#include <stdint.h>
#include <stdio.h>

#include "eecs_388_lib.h"

static const uint8_t steering_angles[] = {10, 25, 75, 45, 100, 40, 125, 15, 150, 50, 170};
static const unsigned long steering_interval_ms = 1000UL;
static Servo steering_servo;
static uint8_t steering_index = 0;
static unsigned long last_steering_update = 0;

static void update_steering()
{
unsigned long now = millis();
if (now - last_steering_update >= steering_interval_ms)
    {
    steering_index = (steering_index + 1) % (sizeof(steering_angles) / sizeof(steering_angles[0]));
    steering_servo.write(steering_angles[steering_index]);
    last_steering_update = now;
    }
}

static char ser_read_with_steering()
{
while (!(UCSR0A & (1 << RXC0)))
    {
    update_steering();
    }
return ser_read();
}

void setup() 
{
gpio_mode(GPIO_13, GPIO_OUTPUT);
gpio_mode(GPIO_12, GPIO_OUTPUT);
gpio_mode(GPIO_11, GPIO_OUTPUT);   /* ADDED (Task 1.5, blue LED) */

steering_servo.attach(GPIO_9);
steering_servo.write(steering_angles[steering_index]);
last_steering_update = millis();

uart_init();

return;
}

void loop() 
{
uint16_t dist = 0;              /* LIDAR distance data is 16 bits. */
uint8_t frame_count =0;
ser_printline("Setup completed.");
ser_write( '\n' );
ser_printf("%u", dist);
while( 1 )
    { 
    update_steering();
    if ('Y' == ser_read_with_steering() && 'Y' == ser_read_with_steering())
        {
            uint8_t dist_l = ser_read_with_steering();   /* byte 3 */
        uint8_t dist_h = ser_read_with_steering();   /* byte 4 */
        dist = ((uint16_t)dist_h << 8) | dist_l;
        uint8_t sum = 0x59 + 0x59 + dist_l + dist_h;   /* ADDED (Task 1.4) */
        
        /* CHANGED (Task 1.4): was a loop that read and discarded 5 bytes */
        for (uint8_t i = 0; i < 4; i++)      /* bytes 5-8 */
            {
            sum += ser_read_with_steering();
            }
        uint8_t chk_byte = ser_read_with_steering();       /* byte 9 */
        bool chk_ok = (sum == chk_byte);     /* uint8_t wraps = low byte */
//test for sending distance and checksum to serial port
        if (chk_ok)
            {
            uint8_t packet_sum = 0xA5 + 0x5A + dist_l + dist_h;
            ser_write((char)0xA5);
            ser_write((char)0x5A);
            ser_write((char)dist_l);
            ser_write((char)dist_h);
            ser_write((char)packet_sum);
            }

        /* Task 1.3: 75 cm threshold per the handout */

        /*FINAL PROJECT task 2: greater than 200 cm is green, 
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
        if (++frame_count >= 10) 
            {
            frame_count = 0;
            ser_printf("%ucm", dist);
            //ser_printf("dist = %u cm", dist);
            ser_printf("checksum %s", chk_ok ? "OK" : "BAD");   /* ADDED */
            }

        }
        
    }

return;
}