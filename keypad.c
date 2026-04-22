#include "keypad.h"

#define KEYPAD_PORT GPIOC

static uint8_t col_pins[4] = {4, 5, 8, 9};
static uint8_t row_pins[4] = {0, 1, 2, 3};

static char keymap[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

extern void delay_us(uint32_t us);

void keypad_init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

    KEYPAD_PORT->MODER &= ~(0x000F0FFF);
    KEYPAD_PORT->PUPDR &= ~(0x000F0FFF);

    KEYPAD_PORT->MODER |= (0x55);

    KEYPAD_PORT->PUPDR |= (1 << 8) | (1 << 10) | (1 << 16) | (1 << 18);

    KEYPAD_PORT->ODR |= 0x0F;
}

char keypad_scan(void) {
    for (int r = 0; r < 4; r++) {
        KEYPAD_PORT->ODR |= 0x0F;
        KEYPAD_PORT->ODR &= ~(1 << row_pins[r]);

        delay_us(50);

        for (int c = 0; c < 4; c++) {
            if (!(KEYPAD_PORT->IDR & (1 << col_pins[c]))) {
                delay_us(10000);
                while (!(KEYPAD_PORT->IDR & (1 << col_pins[c])));
                return keymap[r][c];
            }
        }
    }
    return 0;
}
