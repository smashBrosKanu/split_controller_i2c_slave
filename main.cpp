#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <cstdio>  // Add this line

#define I2C_ADDR  0x01            // Define the I2C address
#define I2C_SDA   0               // GP0 is used for SDA
#define I2C_SCL   1               // GP1 is used for SCL

#define BUTTON_PIN_1  14
#define BUTTON_PIN_2  15

uint8_t buttonState = 0;
uint8_t lastButtonState = 0;

void requestEvent()
{
    if (buttonState != lastButtonState)
    {
        int writtenBytes = i2c_write_blocking(i2c0, I2C_ADDR, &buttonState, 1, false);
        if (writtenBytes != 1) {
            printf("Error: i2c_write_blocking failed to write all bytes.\n");
        }
        lastButtonState = buttonState;
    }
}

int main()
{
    stdio_init_all();  // Add this line

    // I2C setup
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    i2c_set_slave_mode(i2c0, true, I2C_ADDR);

    // Button setup
    gpio_init(BUTTON_PIN_1);
    gpio_set_dir(BUTTON_PIN_1, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_1);
    gpio_init(BUTTON_PIN_2);
    gpio_set_dir(BUTTON_PIN_2, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_2);

    while (true)
    {
        buttonState = 0;
        if (gpio_get(BUTTON_PIN_1) == 0)
        {
            buttonState |= 1 << 0;
        }
        if (gpio_get(BUTTON_PIN_2) == 0)
        {
            buttonState |= 1 << 1;
        }

        if (buttonState != lastButtonState)
        {
            requestEvent();
        }
    }

    return 0;
}