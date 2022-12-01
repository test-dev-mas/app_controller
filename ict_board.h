#ifndef _ICT_BOARD_H
#define _ICT_BOARD_H

extern uint8_t test_points[64];
void init_ict_board();
void rt67_on();
void rt67_off();
void rt68_on();
void rt68_off();
void relay_call(uint8_t address);

#endif