#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979f
#endif

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
int current_cycle_index = 0;

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
 * Dispatches to the selected animation when in ANIMATED mode
*/
void set_animated();

/**
 * Advances the cycle index and returns the next animation to play
*/
int advance_cycle();

/**
 * Sets clock time using lazy animation
*/
void set_lazy();

/**
 * Sets clock time using chaos animation
*/
void set_chaos();

/**
 * Sets clock time using waves animation
*/
void set_waves();

/**
 * Sets clock time using circle animation (synchronized spinning discs)
*/
void set_circle();

/**
 * Sets clock time using spiral animation (diagonal wave sweep)
*/
void set_spiral();

/**
 * Sets clock time using attraction animation (digits move to attraction vector field)
*/
void set_attract();

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
      case ANIMATED:
        set_animated();
        break;
    }
  }
}

void set_animated()
{
  int anim = get_clock_animation();
  if (anim == CYCLE)
    anim = advance_cycle();
  switch(anim)
  {
    case WAVE:   set_waves();  break;
    case CHAOS:    set_chaos();    break;
    case CIRCLE: set_circle(); break;
    case ATTRACT: set_attract(); break;
    case SPIRAL: set_spiral(); break;
  }
}

int advance_cycle()
{
  int anim;
  if (get_cycle_type() == RANDOM_ORDER)
  {
    uint32_t seed = (uint32_t)(hour() * 60 + minute());
    anim = (int)((seed * 2654435761UL) % 5);
  }
  else
  {
    anim = current_cycle_index;
    current_cycle_index = (current_cycle_index + 1) % 5;
  }
  return anim;
}

void set_lazy()
{
  set_speed(200);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

void set_chaos()
{
  set_speed(400);
  set_acceleration(150);
  set_direction(CLOCKWISE2);
  set_clock_time(last_hour, last_minute);
}

static float circle_raw_angle(int col, int row, int hand_idx)
{
    const float cx = 4.5f, cy = 2.0f;
    const float max_dist = sqrtf(3.5f * 3.5f + 1.0f * 1.0f);
    const float inner_angle = 70.0f, outer_angle = 80.0f;

    float x = (float)(col + 1), y = (float)(row + 1);
    float dx = cx - x, dy = cy - y;
    float theta = atan2f(dy, dx) * 180.0f / (float)M_PI;
    float r_norm = fminf(sqrtf(dx*dx + dy*dy) / max_dist, 1.0f);
    float offset = inner_angle + (outer_angle - inner_angle) * r_norm;

    return (hand_idx == 0) ? theta + offset : 360.0f + theta - offset;
}

static uint16_t circle_fw_angle(float js_angle)
{
    float a = fmodf(js_angle, 360.0f);
    if (a < 0.0f) a += 360.0f;
    int norm = (int)roundf(a);
    if (norm == 360) norm = 0;
    return (uint16_t)((360 - norm) % 360);
}

void set_circle()
{
    const unsigned long P1_WAIT_MS = 8000UL;
    const int           P2_RIPPLE  = 800;
    const unsigned long P2_WAIT_MS = 17000UL;
    const int           P3_RIPPLE  = 400;

    t_half_digitl vec[8];
    for (int hd = 0; hd < 8; hd++)
        for (int r = 0; r < 3; r++) {
            vec[hd].clocks[r].angle_h = circle_fw_angle(circle_raw_angle(hd, r, 0));
            vec[hd].clocks[r].angle_m = circle_fw_angle(circle_raw_angle(hd, r, 1));
        }

    // Phase 1: move all clocks to vector field positions (shortest path)
    set_speed(800);
    set_acceleration(150);
    set_direction(MIN_DISTANCE);
    for (int hd = 0; hd < 8; hd++)
        set_half_digit(hd, vec[hd]);

    { unsigned long t = millis() + P1_WAIT_MS;
      while (millis() < t) { update_MDNS(); handle_webclient();
                             if (is_ota_in_progress()) return; delay(100); } }

    // Phase 2: ripple spin center-outward, left half CW, right half CCW
    set_speed(800);
    set_acceleration(150);
    static const int left[]  = {3, 2, 1, 0};
    static const int right[] = {4, 5, 6, 7};
    for (int g = 0; g < 4; g++) {
        set_direction(CLOCKWISE3);
        set_half_digit(left[g],  vec[left[g]]);
        set_direction(COUNTERCLOCKWISE3);
        set_half_digit(right[g], vec[right[g]]);
        if (g < 3) delay(P2_RIPPLE);
    }

    { unsigned long t = millis() + P2_WAIT_MS;
      while (millis() < t) { update_MDNS(); handle_webclient();
                             if (is_ota_in_progress()) return; delay(100); } }

    // Phase 3: settle to current time
    set_speed(400);
    set_acceleration(100);
    set_direction(MIN_DISTANCE);
    t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);
    for (int i = 0; i < 8; i++) {
        set_half_digit(i, clock.digit[i/2].halfs[i%2]);
        delay(P3_RIPPLE);
    }
}

void set_spiral()
{
  /*TODO*/
}

void set_attract()
{
  /*TODO*/
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