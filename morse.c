#include "morse.h"
#include <stdint.h>

extern void delay_ms(uint32_t ms);

#define DOT_TIME    500
#define DASH_TIME   (DOT_TIME * 3)
#define SYMBOL_GAP  DOT_TIME
#define LETTER_GAP  (DOT_TIME * 3)

static const char* morse_alpha[] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
    "-.-", ".-..", "--", "-.", "---", ".---", "--.-", ".-.", "...", "-",
    "..-", "...-", ".--", "-..-", "-.--", "--.."
};
static const char* morse_digit[] = {
    "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."
};

void morse_blink(const char* pattern) {
    for (int i = 0; pattern[i] != '\0'; i++) {
        GPIOA->BSRR = (1 << 5);
        if (pattern[i] == '.') delay_ms(DOT_TIME);
        else delay_ms(DASH_TIME);

        GPIOA->BSRR = (1 << (5 + 16));
        delay_ms(SYMBOL_GAP);
    }
}

void morse_send_string(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            morse_blink(morse_alpha[str[i] - 'A']);
        } else if (str[i] >= '0' && str[i] <= '9') {
            morse_blink(morse_digit[str[i] - '0']);
        }
        delay_ms(LETTER_GAP);
    }
}
