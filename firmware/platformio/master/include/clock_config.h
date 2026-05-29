#ifndef CLOCK_CONFIG_H
#define CLOCK_CONFIG_H

#include <Preferences.h>

/** 
 * Clock connection's modes
 */
enum wireless_modes
{
  HOTSPOT,
  EXT_CONN
};

/**
 * Clock operational modes
 */
enum clock_modes
{
  OFF = 0,
  LAZY = 1,
  ANIMATED = 2
};

/**
 * Clock animation types (used when mode is ANIMATED)
 */
enum clock_animations
{
  WAVE = 0,
  FUN = 1,
  CIRCLE = 2,
  SPIRAL = 3,
  CYCLE = 4
};

/**
 * Cycle order (used when animation is CYCLE)
 */
enum cycle_types
{
  SEQUENTIAL = 0,
  RANDOM_ORDER = 1
};

/**
 * Load configuration from the EEPROM
 */
void begin_config();

/**
 * Clear EEPROM configuration
 */
void clear_config();

/**
 * Closes the preferencies object
 */
void end_config();

/**
 * Get current clock mode
 */
int get_clock_mode();

/**
 * Get current animation (used when mode is ANIMATED)
 */
int get_clock_animation();

/**
 * Get current cycle type (SEQUENTIAL or RANDOM_ORDER)
 */
int get_cycle_type();

/**
 * Gets current sleep time at a given day and hour
 * @param day   day of the week
 * @param hour  hour of the day
 */
bool get_sleep_time(int day, int hour);

/**
 * Gets current connection mode
 */
int get_connection_mode();

/**
 * Gets current time zone based on UTC offset
 */
int get_timezone();

/**
 * Gets current SSID
 */
char *get_ssid();

/**
 * Gets current password
 */
char *get_password();

/**
 * Sets clock mode
 * @param value   mode value of type clock_modes
 */
void set_clock_mode(int value);

/**
 * Sets animation type (used when mode is ANIMATED)
 * @param value   animation value of type clock_animations
 */
void set_clock_animation(int value);

/**
 * Sets cycle type
 * @param value   cycle type value of type cycle_types
 */
void set_cycle_type(int value);

/**
 *  Sets current sleep time at a given day and hour
 * @param day   day of the week
 * @param hour  hour of the day
 * @param value true if clock is  disabled, false otherwise
 */
void set_sleep_time(int day, int hour, bool value);

/**
 *  Saves sleep time array on EEPROM
 */
void save_sleep_time();

/**
 *  Sets connection mode
 * @param value   mode value of type wireless_modes
 */
void set_connection_mode(int value);

/**
 *  Sets the time zone
 * @param value   time zone based on UTC offset
 */
void set_timezone(int value);

/**
 *  Sets SSID value
 * @param value   SSID string
 */
void set_ssid(const char *value);

/**
 *  Sets password value
 * @param value   password string
 */
void set_password(const char *value);

/**
 *  Gets OTA update password
 */
const char *get_ota_password();

/**
 *  Sets OTA update password
 * @param value   OTA password string
 */
void set_ota_password(const char *value);

#endif