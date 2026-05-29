#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>

#include "i2c.h"
#include "clock_state.h"
#include "clock_manager.h"
#include "digit.h"
#include "wifi_utils.h"
#include "web_server.h"
#include "clock_config.h"
#include "ntp.h"


int last_hour = -1;
int last_minute = -1;
bool is_stopped = false;

#define I2C_SCAN_INTERVAL_MS 5000
unsigned long last_i2c_scan_ms = 0;

constexpr int I2C_SDA_PIN = 21;
constexpr int I2C_SCL_PIN = 22;
constexpr uint32_t I2C_CLOCK_HZ = 100000;
constexpr uint8_t I2C_FIRST_SLAVE_ADDR = 0x08;
constexpr uint8_t I2C_LAST_SLAVE_ADDR = 0x0F;

/**
 * Sets clock to the current time
*/
void set_time();

/**
 * Sets clock time using lazy animation
*/
void set_lazy();

/**
 * Sets clock time using fun animation
*/
void set_fun();

/**
 * Sets clock time using waves animation
*/
void set_waves();

/**
 * Sets clock to stop state
*/
void stop();

/**
 * Custom delay to update web clients
 * @param value   time in milliseconds
*/
void _delay(int value);

/**
 * Scans all I2C addresses and prints found devices to Serial
*/
void i2c_scan();

/**
 * Tries to release a stuck I2C bus by clocking SCL and issuing a STOP.
 */
void i2c_recover_bus();

/**
 * Initializes I2C with explicit pins and a timeout to avoid setup lockups.
 */
void i2c_begin_safe();

void setup() {

  

  Serial.begin(115200);
  Serial.println("\nClockClock24 by marino222");
  delay(3000);
  // Load configuration from EEPROM
  begin_config();
  pinMode(LED_BUILTIN, OUTPUT);

  

  if(get_connection_mode() == HOTSPOT)
    wifi_create_AP("ClockClock 24", "clock");
  else if( !wifi_connect(get_ssid(), get_password(), "clock") )
  {
    set_connection_mode(HOTSPOT);
    wifi_create_AP("ClockClock 24", "clock");
  }

  if(get_connection_mode() == EXT_CONN)
  {
    // Initialize NTP
    begin_NTP();
    setSyncProvider(get_NTP_time);
    // Sync every 30 minutes
    setSyncInterval(60 * 30);
  }

  // Bring up I2C after WiFi so the device is still reachable if the bus is faulty.
  i2c_begin_safe();
  i2c_scan();

  // Starts web server
  server_start();
}

void loop() {

  update_MDNS();
  handle_webclient();

  if (is_ota_in_progress()) return;

  if(get_connection_mode() == HOTSPOT && is_time_changed_browser())
  {
    t_browser_time browser_time = get_browser_time();
    setTime(browser_time.hour, 
      browser_time.minute, 
      browser_time.second, 
      browser_time.day, 
      browser_time.month,  
      browser_time.year);
  }

  if(get_connection_mode() == EXT_CONN && get_timezone() != get_ntp_timezone())
  {
    set_ntp_timezone(get_timezone());
    setSyncProvider(get_NTP_time);
  }

  get_clock_mode() != OFF ? set_time() : stop();

  if (millis() - last_i2c_scan_ms >= I2C_SCAN_INTERVAL_MS)
  {
    last_i2c_scan_ms = millis();
    i2c_scan();
  }
}

void set_time()
{
  int day_week = (weekday() + 5) % 7;
  if(get_sleep_time(day_week, hour()))
    stop();
  else if(hour() != last_hour || minute() != last_minute)
  {
    is_stopped = false;
    last_hour = hour();
    last_minute = minute();
    switch(get_clock_mode())
    {
      case LAZY:
        set_lazy();
        break;
      case FUN:
        set_fun();
        break;
      case WAVES:
        set_waves();
        break;
    }
  }
}

void set_lazy()
{
  set_speed(200);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

void set_fun()
{
  set_speed(400);
  set_acceleration(150);
  set_direction(CLOCKWISE2);
  set_clock_time(last_hour, last_minute);
}

void set_waves()
{
  set_speed(800);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock(d_IIII);
  _delay(9000);
  set_speed(400);
  set_acceleration(100);
  set_direction(CLOCKWISE2);
  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);
  for (int i = 0; i <8; i++)
  {
    set_half_digit(i, clock.digit[i/2].halfs[i%2]);
    delay(400);
  }
}

void stop()
{
  if(!is_stopped)
  {
    is_stopped = true;
    last_hour = -1;
    last_minute = -1;
    set_direction(MIN_DISTANCE);
    set_speed(300);
    set_acceleration(150);
    set_clock(d_stop);
  }
}

void i2c_scan()
{
  Serial.println("\n--- I2C Scanner ---");
  int found = 0;
  for (uint8_t addr = I2C_FIRST_SLAVE_ADDR; addr <= I2C_LAST_SLAVE_ADDR; addr++)
  {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0)
    {
      Serial.printf("  Found device at 0x%02X (decimal %d)\n", addr, addr);
      found++;
    }
  }
  if (found == 0)
    Serial.println("  No I2C devices found!");
  Serial.printf("--- Scan done, %d device(s) found ---\n\n", found);
}

void i2c_recover_bus()
{
  pinMode(I2C_SDA_PIN, INPUT_PULLUP);
  pinMode(I2C_SCL_PIN, INPUT_PULLUP);

  const bool sda_high = digitalRead(I2C_SDA_PIN) == HIGH;
  const bool scl_high = digitalRead(I2C_SCL_PIN) == HIGH;
  if (sda_high && scl_high)
    return;

  Serial.println("[I2C] Bus busy at boot, trying recovery pulses...");

  pinMode(I2C_SCL_PIN, OUTPUT_OPEN_DRAIN);
  digitalWrite(I2C_SCL_PIN, HIGH);
  delayMicroseconds(5);

  for (int i = 0; i < 9; i++)
  {
    digitalWrite(I2C_SCL_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(I2C_SCL_PIN, HIGH);
    delayMicroseconds(5);
  }

  // STOP condition: SDA low -> SCL high -> SDA high.
  pinMode(I2C_SDA_PIN, OUTPUT_OPEN_DRAIN);
  digitalWrite(I2C_SDA_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(I2C_SCL_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(I2C_SDA_PIN, HIGH);
  delayMicroseconds(5);

  pinMode(I2C_SDA_PIN, INPUT_PULLUP);
  pinMode(I2C_SCL_PIN, INPUT_PULLUP);
}

void i2c_begin_safe()
{
  i2c_recover_bus();
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_CLOCK_HZ);
  Wire.setTimeOut(25);
}

void _delay(int value)
{
  for (int i = 0; i <value/100; i++)
  {
    update_MDNS();
    handle_webclient();
    if (is_ota_in_progress()) return;
    delay(value/100);
  }
}