#include <stdint.h>
#include <avr/io.h>

#include "ict_board.h"

#define RT67_ON     PORTD|=(1<<PD7);
#define RT67_OFF    PORTD&=~(1<<PD7);
#define RT68_ON     PORTB|=(1<<PB7);
#define RT68_OFF    PORTB&=~(1<<PB7);

/* same order as the RTx label on PCB */
uint8_t test_points[64] = {
    0x70, 0x78, 0x74, 0x7C, 0x72, 0x7A, 0x76, 0x7E, 0x71, 0x79, 0x75, 0x7D, 0x73, 0x7B, 0x77, 0x7F,\
    0xB0, 0xB8, 0xB4, 0xBC, 0xB2, 0xBA, 0xB6, 0xBE, 0xB1, 0xB9, 0xB5, 0xBD, 0xB3, 0xBB, 0xB7, 0xBF,\
    0xD0, 0xD8, 0xD4, 0xDC, 0xD2, 0xDA, 0xD6, 0xDE, 0xD1, 0xD9, 0xD5, 0xDD, 0xD3, 0xDB, 0xD7, 0xDF,\
    0xE0, 0xE8, 0xE4, 0xEC, 0xE2, 0xEA, 0xE6, 0xEE, 0xE1, 0xE9, 0xE5, 0xED, 0xE3, 0xEB, 0xE7, 0xEF\
};

void init_ict_board() {
    DDRE |= (1 << PE3) | (1 << PE4) | (1 << PE5);                   // D5/2/3 selects HC154
    PORTE |= (1 << PE3) | (1 << PE4) | (1 << PE5);                  // pull high to disable all HC154             

    DDRG |= (1 << PG5);                                             // D4
    PORTG |= (1 << PG5);                                            // pull high to disable all HC154

    DDRH |= (1 << PH3) | (1 << PH4) | (1 << PH5) | (1 << PH6);      // A0/1/2/3 on HC154
    PORTH |= (1 << PH3) | (1 << PH4) | (1 << PH5) | (1 << PH6);

    DDRB |= (1 << PB6) | (1 << PB7);                                // RT68 (MULTIMETER) COIL
    PORTB|= (1<<PB7);                                               // multimeter shuold be on voltage setting upon start-up

    /*  RT67 (optional)
        can be controlled by a spare GPIO, usually on TB5/6/7
        uncomment rt67_on()/rt67_off() definitions below, update GPIO!
    */
    DDRD |= (1 << PD7);                                             // PD7 is D38 (TB5.10)
}

void rt67_on() {
    PORTD|=(1<<PD7);
}

void rt67_off() {
    PORTD&=~(1<<PD7);
}

/* switch to V measurement if multimeter is connected */
void rt68_on() {
    PORTB|=(1<<PB7);
}

/* switch to R measurement if multimeter is connected */
void rt68_off() {
    PORTB&=~(1<<PB7);
}

void relay_call(uint8_t address) {
    (address & 0x80) ? (PORTE |= (1 << PE4)) : (PORTE &= ~(1 << PE4));          // U1
    (address & 0x40) ? (PORTE |= (1 << PE5)) : (PORTE &= ~(1 << PE5));          // U2
    (address & 0x20) ? (PORTG |= (1 << PG5)) : (PORTG &= ~(1 << PG5));          // U3
    (address & 0x10) ? (PORTE |= (1 << PE3)) : (PORTE &= ~(1 << PE3));          // U4
    (address & 0x08) ? (PORTH |= (1 << PH3)) : (PORTH &= ~(1 << PH3));          // A0
    (address & 0x04) ? (PORTH |= (1 << PH4)) : (PORTH &= ~(1 << PH4));          // A1
    (address & 0x02) ? (PORTH |= (1 << PH5)) : (PORTH &= ~(1 << PH5));          // A2
    (address & 0x01) ? (PORTH |= (1 << PH6)) : (PORTH &= ~(1 << PH6));          // A3
}