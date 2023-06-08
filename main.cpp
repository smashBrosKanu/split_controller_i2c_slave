#include <i2c_fifo.h>
#include <i2c_slave.h>
#include "pico/stdlib.h"
//#include "hardware/i2c.h"
#include <cstdio>  // Add this line
#include "tusb.h"

#define I2C_SLAVE_ADDRESS  0x17            // Define the I2C address
#define I2C_SDA   0               // GP0 is used for SDA
#define I2C_SCL   1               // GP1 is used for SCL

// Define the pins for each button
//#define BUTTON_PIN_COUNT 24
//const uint BUTTON_PINS[BUTTON_PIN_COUNT] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 26, 27, 28 };
#define BUTTON_PIN_COUNT 18

const uint BUTTON_PINS[BUTTON_PIN_COUNT] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 26, 27, 28, 29 };

uint32_t buttonState = 0;
uint32_t lastButtonState = 0;

static struct
{
    uint8_t mem[256];
    uint8_t mem_address;
    uint8_t num;
    bool mem_address_written;
} context;

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t* i2c, i2c_slave_event_t event) {
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
        if (!context.mem_address_written) {
            // writes always start with the memory address
            context.mem_address = i2c_read_byte(i2c);
            context.mem_address_written = true;
        }
        else {
            // save into memory
            context.mem[context.mem_address] = i2c_read_byte(i2c);
            context.mem_address++;
        }
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data
        if (context.num > 3)
        {
            context.num = 0;
        }
        i2c_write_byte(i2c, context.mem[context.num]);
        context.num++;
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        context.mem_address_written = false;
        break;
    default:
        break;
    }
}

void requestEvent()
{
    if (buttonState != lastButtonState)
    {
        context.mem[0] = buttonState & 0xff;
        context.mem[1] = (buttonState >> 8) & 0xff;
        context.mem[2] = (buttonState >> 16) & 0xff;
        context.mem[3] = (buttonState >> 24) & 0xff;
        lastButtonState = buttonState;
    }
}

int main()
{
    stdio_init_all();  // Add this line
    tusb_init();

    // I2C setup
    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, &i2c_slave_handler);

    // Button setup
    for (int i = 0; i < BUTTON_PIN_COUNT; ++i) {
        gpio_init(BUTTON_PINS[i]);
        gpio_set_dir(BUTTON_PINS[i], GPIO_IN);
        gpio_pull_up(BUTTON_PINS[i]);
    }

    uint32_t mask = 0xFFFFFFFF; // All bits are 1.

    // Clear only the bits corresponding to BUTTON_PINS.
    for (int i = 0; i < BUTTON_PIN_COUNT; ++i) {
        mask &= ~(1U << BUTTON_PINS[i]);
    }

    while (true)
    {
        buttonState = gpio_get_all();
        buttonState = buttonState | mask;

        // Bit setting/copying code starts here
        for (int i = 0; i < 3; i++) {
            uint32_t bit1 = 1U;  // Always set bit to 1
            uint32_t bit2 = (buttonState >> (7 + i)) & 1U;  // Extract the (6+i)th bit

            // Set the (6+i)th bit to 1
            buttonState |= (bit1 << (7 + i));

            // Set the (16+i)th bit to the value of the (6+i)th bit
            if (bit2 == 0) {
                buttonState &= ~(1U << (16 + i));  // If bit is 0, clear the bit
            }
            else {
                buttonState |= (1U << (16 + i));  // If bit is 1, set the bit
            }
        }

        if (buttonState != lastButtonState)
        {
            context.mem[0] = buttonState & 0xff;
            context.mem[1] = (buttonState >> 8) & 0xff;
            context.mem[2] = (buttonState >> 16) & 0xff;
            context.mem[3] = (buttonState >> 24) & 0xff;
            lastButtonState = buttonState;

            // Add the following lines:
            // Print each bit one by one
            for (int i = 31; i >= 0; --i) {
                printf("%d", (buttonState >> i) & 1);
            }
            printf("\n");
            fflush(stdout);  // Make sure the output is printed immediately
        }

        //sleep_ms(500);

    }

    return 0;
}

