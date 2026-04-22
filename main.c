#include "stm32l476xx.h"
#include <stdint.h>
#include "keypad.h"
#include "encoder.h"
#include "morse.h"

#define LCD_PORT GPIOB

#define LCD_RS   0
#define LCD_E    1

#define LCD_D4   6   // PB6
#define LCD_D5   7   // PB7
#define LCD_D6   8   // PB8
#define LCD_D7   9   // PB9

#define LED_PORT GPIOA
#define LED_PIN  5

uint32_t SystemCoreClock = 4000000;
static uint32_t ticks_per_us = 0;
int cursor_pos = 0;
char message_buffer[33];
int msg_idx = 0;

void timer_init(void) {
	ticks_per_us = SystemCoreClock/1000000;
}

void delay_us(uint32_t us) {
	SysTick->LOAD = (us*ticks_per_us) - 1;
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

	while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));

	SysTick->CTRL = 0;
}

void delay_ms(uint32_t ms) {
	while(ms--) {
		delay_us(1000);
	}
}

static inline void pin_set(uint32_t pin) {
    LCD_PORT->BSRR = (1 << pin);
}

static inline void pin_clr(uint32_t pin) {
    LCD_PORT->BSRR = (1 << (pin + 16));
}

static void lcd_pulse_enable(void) {
	delay_us(10);
    pin_set(LCD_E);
    delay_us(20);
    pin_clr(LCD_E);
    delay_us(100);
}

static void lcd_write_nibble(uint8_t nibble) {
    LCD_PORT->BSRR = (0xF << (LCD_D4 + 16));

    if(nibble & 0x01) pin_set(LCD_D4); else pin_clr(LCD_D4);
    if(nibble & 0x02) pin_set(LCD_D5); else pin_clr(LCD_D5);
    if(nibble & 0x04) pin_set(LCD_D6); else pin_clr(LCD_D6);
    if(nibble & 0x08) pin_set(LCD_D7); else pin_clr(LCD_D7);

    lcd_pulse_enable();
}

static void lcd_write_byte(uint8_t data) {
    pin_clr(LCD_RS);
    lcd_write_nibble(data >> 4);
    lcd_write_nibble(data & 0x0F);
    delay_us(50);
}

static void lcd_send_data(uint8_t data) {
    pin_set(LCD_RS);
    lcd_write_nibble(data >> 4);
    lcd_write_nibble(data & 0x0F);
    delay_us(50);
}

void lcd_print_char(char c) {
    if (cursor_pos == 16) {
        lcd_write_byte(0xC0);
    }

    if (cursor_pos < 32) {
        lcd_send_data(c);
        cursor_pos++;
    }
}

void lcd_backspace(void) {
    if (cursor_pos <= 0) return;

    cursor_pos--;

    if (cursor_pos == 15) {
        lcd_write_byte(0x80 + 15);
    }
    else {
        lcd_write_byte(0x10);
    }

    lcd_send_data(' ');

    if (cursor_pos == 15) {
        lcd_write_byte(0x80 + 15);
    } else {
        lcd_write_byte(0x10);
    }
}
	void lcd_init(void) {
		delay_ms(100);
		lcd_write_nibble(0x03);
		delay_ms(10);
		lcd_write_nibble(0x03);
		delay_ms(1);
		lcd_write_nibble(0x03);
		delay_ms(1);

		lcd_write_nibble(0x02);
		delay_ms(5);

		lcd_write_byte(0x28);
		delay_ms(1);
		lcd_write_byte(0x0C);
		delay_ms(1);
		lcd_write_byte(0x01);
		delay_ms(5);
		lcd_write_byte(0x06);
		delay_ms(1);
	}

void gpio_init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;

    uint8_t lcd_pins[] = {LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7};
    for (int i = 0; i < 6; i++) {
        LCD_PORT->MODER &= ~(3 << (lcd_pins[i]*2));
        LCD_PORT->MODER |=  (1 << (lcd_pins[i]*2));
    }

    LED_PORT->MODER &= ~(3 << (LED_PIN * 2));
    LED_PORT->MODER |=  (1 << (LED_PIN * 2));
}

int main(void) {
	timer_init();
    gpio_init();
    lcd_init();
    keypad_init();

    char first_key = 0;
    int is_second_key = 0;

    while(1) {
    	char key = keypad_scan();
    	if (key != 0) {
    	    if (key == 'C' && !is_second_key) {
    	        lcd_write_byte(0x01);
    	        delay_ms(5);
    	        cursor_pos = 0;

    	        morse_send_string(message_buffer);

    	        msg_idx = 0;
    	        message_buffer[0] = '\0';
    	    }
    	    else if (key == 'D') {
    	        if (is_second_key) {
    	            lcd_backspace();
    	            is_second_key = 0;
    	        } else if (msg_idx > 0) {
    	            lcd_backspace();
    	            msg_idx--;
    	            message_buffer[msg_idx] = '\0';
    	        }
    	    }
    	    else if (key == '#') {
    	        lcd_write_byte(0x01);
    	        cursor_pos = 0;
    	        msg_idx = 0;
    	        message_buffer[0] = '\0';
    	        is_second_key = 0;
    	    }
    	    else {
    	        if (!is_second_key) {
    	            first_key = key;
    	            lcd_print_char(key);
    	            is_second_key = 1;
    	        } else {
    	            char final_char = translate_pair(first_key, key);
    	            lcd_backspace();
    	            lcd_print_char(final_char);

    	            if (msg_idx < 32) {
    	                message_buffer[msg_idx++] = final_char;
    	                message_buffer[msg_idx] = '\0';
    	            }
    	            is_second_key = 0;
    	        }
    	    }
    	}
    }


    while(1);
}
