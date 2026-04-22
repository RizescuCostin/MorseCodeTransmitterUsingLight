#include "encoder.h"

char translate_pair(char first, char second) {
    int offset = 0;

    if (second >= '0' && second <= '9') {
        offset = second - '0';
    } else if (second >= 'A' && second <= 'D') {
        offset = 10 + (second - 'A');
    }

    if (first == '0' || first == '1' || first == '2') {
        int base = (first - '0') * 10;
        char result = 'A' + base + offset;
        return (result <= 'Z') ? result : '?';
    }

    if (first == '*') {
        if (second >= '0' && second <= '9') return second;
    }

    return '?';
}
