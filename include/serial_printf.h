#ifndef SERIAL_PRINTF_H
#define SERIAL_PRINTF_H

#include <Arduino.h>
#include <printf.h>

// this is needed for printf.h
void _putchar(char character) __attribute__((weak));

void _putchar(char character) {
    Serial.write(character);
}

#endif
