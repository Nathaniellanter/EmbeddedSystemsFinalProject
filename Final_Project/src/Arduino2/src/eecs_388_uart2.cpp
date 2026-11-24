/******************************************************************************
 *   Header Includes
 *******************************************************************************/
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <Servo.h>
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

static uint16_t read_distance()
{
    uint8_t previous = 0;

    while (true)
    {
        uint8_t current = (uint8_t)ser_read_with_steering();
        if (previous == 0xA5 && current == 0x5A)
        {
            uint8_t distance_low = (uint8_t)ser_read_with_steering();
            uint8_t distance_high = (uint8_t)ser_read_with_steering();
            uint8_t checksum = (uint8_t)ser_read_with_steering();
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

static void print_distance(uint16_t distance)
{
    char digits[5];
    uint8_t digit_count = 0;

    do
    {
        digits[digit_count++] = (char)('0' + (distance % 10));
        distance /= 10;
    } while (distance > 0);

    ser_write('D');
    ser_write('i');
    ser_write('s');
    ser_write('t');
    ser_write('a');
    ser_write('n');
    ser_write('c');
    ser_write('e');
    ser_write(':');
    ser_write(' ');

    while (digit_count > 0)
    {
        ser_write(digits[--digit_count]);
    }

    ser_printline(" cm");
}

void setup()
{
    uart_init();
    gpio_mode(GPIO_13, GPIO_OUTPUT);
    gpio_mode(GPIO_12, GPIO_OUTPUT);
    gpio_mode(GPIO_11, GPIO_OUTPUT);

    steering_servo.attach(GPIO_9);
    steering_servo.write(steering_angles[steering_index]);
    last_steering_update = millis();

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
    static uint8_t frames_since_print = 0;

    if (++frames_since_print >= 10)
    {
        frames_since_print = 0;
        print_distance(distance);
    }
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

    return;
}
