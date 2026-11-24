/******************************************************************************
 *   Header Includes
 *******************************************************************************/
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include "eecs_388_lib2.h"

/******************************************************************************
 *   Function: setup() - Initializes the Arduino System
 *      Pre condition:
 *          Hardware must be properly connected (LEDS, RX/TX from arduino1)
 *      Post condition:
 *          Runs initialization calls one time on power up
 *			UART is initialized
 *          Input/Output (IO) pins are configured for LEDs
 *******************************************************************************/


static uint16_t read_distance()
{
    uint8_t previous = 0;

    while (true)
    {
        uint8_t current = (uint8_t)ser_read();
        if (previous == 0xA5 && current == 0x5A)
        {
            uint8_t distance_low = (uint8_t)ser_read();
            uint8_t distance_high = (uint8_t)ser_read();
            uint8_t checksum = (uint8_t)ser_read();
            uint8_t expected_checksum =
                (uint8_t)(0xA5 + 0x5A + distance_low + distance_high);

            if (checksum == expected_checksum)
            {
                return ((uint16_t)distance_high << 8) | distance_low;
            }

            previous = (checksum == 0xA5) ? 0xA5 : 0;
        }
        else
        {
            previous = (current == 0xA5) ? 0xA5 : 0;
        }
    }
}

void setup()
{
    uart_init();
    gpio_mode(GPIO_13, GPIO_OUTPUT);
    gpio_mode(GPIO_12, GPIO_OUTPUT);
    gpio_mode(GPIO_11, GPIO_OUTPUT);

    return;
}

/******************************************************************************
 *   Function: loop() - Main execution loop
 *      Pre condition:
 *          setup() has been executed and system is initialized
 *      Post condition:
 *          Performs a single iteration of the system's function
 *          Repeates indefinetely unless the board is reset or powered off
 *******************************************************************************/
void loop()
{
    uint16_t distance = read_distance();

    if (distance > 200)
    {
        gpio_write(GPIO_13, OFF);
        gpio_write(GPIO_12, ON);
        gpio_write(GPIO_11, OFF);
    }
    else if (distance > 100)
    {
        gpio_write(GPIO_13, ON);
        gpio_write(GPIO_12, ON);
        gpio_write(GPIO_11, OFF);
    }
    else if (distance > 60)
    {
        gpio_write(GPIO_13, ON);
        gpio_write(GPIO_12, OFF);
        gpio_write(GPIO_11, OFF);
    }
    else
    {
        static uint8_t flash_state = OFF;
        static unsigned long last_flash_time = 0;
        unsigned long now = millis();

        if (now - last_flash_time >= 100UL)
        {
            flash_state = !flash_state;
            last_flash_time = now;
        }

        gpio_write(GPIO_13, flash_state);
        gpio_write(GPIO_12, OFF);
        gpio_write(GPIO_11, OFF);
    }
    printf("Distance: %d\n", distance);
    return;
}
