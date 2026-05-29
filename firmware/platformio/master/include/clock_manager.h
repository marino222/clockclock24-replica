#ifndef clock_manager_h
#define clock_manager_h

#include "clock_state.h"
#include "i2c.h"
#include "digit.h"
#include "clock_config.h"

/** 
 * Returns current direction
 * @return direction
*/
int get_direction();

/** 
 * Returns current speed
 * @return speed
*/
int get_speed();

/** 
 * Returns current acceleration
 * @return acceleration
*/
int get_acceleration();

/** 
 * Sets current direction
 * @param value     direction
*/
void set_direction(int direction);

/** 
 * Sets current speed
 * @param value     speed
*/
void set_speed(int value);

/** 
 * Sets current acceleration
 * @param value     acceleration
*/
void set_acceleration(int value);

/** 
 * Sends half digit to the specified board
 * @param index         board index (0 <= index < 8)
 * @param half_digit    digit to send
*/
void send_half_digit(int index, t_half_digit half_digit);

/** 
 * Sends a digit to the specified boards
 * @param index         board index (0 <= index < 8)
 * @param half_digit    digit to send
*/
void send_digit(int index, t_digit digit);

/** 
 * Sends the full clock configuration to boards
 * @param full_clock    clock configuration
*/
void send_clock(t_full_clock full_clock);

/** 
 * Converts t_half_digitl to t_half_digit
 * @param lite_digit    t_half_digitl
 * @return t_half_digit
*/
t_half_digit get_full_half_digit(t_half_digitl lite_digit);

/** 
 * Sends the full clock configuration to boards and increments
 * the state counter
 * @param clock_state   clock state
*/
void set_clock(t_full_clock clock_state);

/** 
 * Sends a digit to the specified boards and increments
 * the state counter
 * @param index     digit index (0 <= index < 4)
 * @param digit     digit value
*/
void set_digit(int index, t_digit digit);

/** 
 * Sends a half digit to the specified board and increments
 * the state counter
 * @param index     digit index (0 <= index < 8)
 * @param half      hlaf digit value
*/
void set_half_digit(int index, t_half_digitl half);

/**
 * Sets the specified time on the clock
 * @param h     hour
 * @param m     minute
*/
void set_clock_time(int h, int m);

/**
 * Sets all 24 clocks with per-clock angle pairs using a shared speed/accel/direction.
 * angles[i] = {hour_hand_angle, minute_hand_angle} for clock i (0-23).
 * Clock order: digit 0 half 0 clocks 0-2, digit 0 half 1 clocks 0-2, ..., digit 3 half 1 clocks 0-2.
 */
void set_custom_clock(const uint16_t angles[24][2], int speed, int accel, int direction);

/** 
 * Returns a full clock state from time
 * @param h     hour
 * @param m     minute
 * @return full clock state
*/
t_full_clock get_clock_state_from_time(int h, int m);

/** 
 * @param clock_index   single clock index (0 <= index < 24)
 * @param h_amount      hours hand degree of adjustment
 * @param m_amount      minutes hand degree of adjustment
*/
void adjust_hands(int clock_index, int h_amount, int m_amount);

/**
 * Runs a full-board test locally on the controller.
 * The clock is first moved into the known stop pose, then each selected hand
 * performs exactly one full clockwise rotation and lands back in that pose.
 *
 * @param test_hour     whether to test hour hands
 * @param test_minute   whether to test minute hands
 * @param step_delay_ms pause between individual clock commands
 * @param phase_delay_ms pause between hour and minute phases
*/
void run_full_board_test(bool test_hour, bool test_minute, int step_delay_ms, int phase_delay_ms);

#endif