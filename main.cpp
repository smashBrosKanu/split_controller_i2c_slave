#include <i2c_fifo.h>
#include <i2c_slave.h>
#include "pico/stdlib.h"
//#include "hardware/i2c.h"
#include <cstdio>  // Add this line
#include "tusb.h"

#define I2C_SLAVE_ADDRESS  0x01            // Define the I2C address
#define I2C_SDA   0               // GP0 is used for SDA
#define I2C_SCL   1               // GP1 is used for SCL

// Define the pins for each button
#define BUTTON_PIN_COUNT 13
const uint BUTTON_PINS[BUTTON_PIN_COUNT] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 14, 15, 26, 27 };

uint8_t buttonState = 0;
uint8_t lastButtonState = 0;

static struct
{
    uint8_t mem[256];
    uint8_t mem_address;
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
        // load from memory
        i2c_write_byte(i2c, context.mem[0]);
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
        context.mem[0] = buttonState & 0xff;  // Save the lower byte of buttonState
        context.mem[1] = (buttonState >> 8) & 0xff;  // Save the upper byte of buttonState
        lastButtonState = buttonState;
    }
}

int main()
{
    stdio_init_all();  // Add this line
    tusb_init();

    // I2C setup
    i2c_init(i2c0, 100 * 1000);
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

    while (true)
    {
        buttonState = 0;
        for (int i = 0; i < BUTTON_PIN_COUNT; ++i)
        {
            if (gpio_get(BUTTON_PINS[i]) == 0)
            {
                buttonState |= 1 << i;
            }
        }

        if (buttonState != lastButtonState)
        {
            requestEvent();
            // Add the following lines:
            printf("Button 1 state: %s\n", (buttonState & (1 << 0)) ? "Pressed" : "Released");
            printf("Button 2 state: %s\n", (buttonState & (1 << 1)) ? "Pressed" : "Released");
            fflush(stdout);  // Make sure the output is printed immediately
        }

    }

    return 0;
}