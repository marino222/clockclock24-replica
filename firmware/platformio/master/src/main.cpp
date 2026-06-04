#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <math.h>
#include <algorithm>

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

/**
 * Returns the outward-ripple delay (ms) for a clock at grid position (x, y).
 * x: 1 to 8 (column), y: 1 to 3 (row).
 */
int calculate_start_delay(int x, int y, float speed_multiplier)
{
  float dx = 4.5f - x;
  float dy = 2.0f - y;
  float distance = sqrt(dx * dx + dy * dy);
  float d = speed_multiplier * (distance - 0.5f);
  return d > 0 ? (int)d : 0;
}


/**
 * @brief Calculates the dynamic offset angle (theta) for a vector pair to create an attractor field illusion.
 *
 * This function computes the required angular offset between the inward-pointing 
 * radial line (from the current grid index to the array center) and the physical 
 * unit vectors. By linearly interpolating between an inner and outer angular bound 
 * based on the Euclidean distance from the center, it generates a sweeping "V-shape" 
 * that transitions from a sharp inward pinch near the center to a wide, circular 
 * tangent near the edges of the grid.
 * * Use the bound parameters to control the shape (softness/pointiness) of the attractor field
 *
 * @param x             The horizontal index in the array grid (e.g., 1 to 8).
 * @param y             The vertical index in the array grid (e.g., 1 to 3).
 * @param inner_bound   The minimum offset angle applied at the exact center. Smaller values create a tighter V-shape.
 * @param outer_bound   The maximum offset angle applied at the maximum radius. Values near 90 degrees create a circular tangent.
 * @return float        The calculated offset angle (theta). To get the absolute angles for the physical motors, add and subtract 
 * this value from the base inward radial angle.
 */
float calculateAttractorAngle(float x, float y, float inner_bound, float outer_bound) {
  const float centerX = 4.5f;
  const float centerY = 2.0f;
  
  // Pre-calculated maximum distance from center (4.5, 2.0) to corner (1.0, 1.0)
  // sqrt((3.5 * 3.5) + (1.0 * 1.0))
  const float maxDistance = 3.64005f; 

  float dx = x - centerX;
  float dy = y - centerY;
  float distance = sqrtf(dx * dx + dy * dy);

  // Normalize distance to [0,1], clamped to prevent minor floating point over-extensions
  float t = distance / maxDistance; 
  if (t > 1.0f) t = 1.0f;

  return inner_bound + t * (outer_bound - inner_bound);
}

/**
 * @brief Calculates the absolute target angle for a specific motor in the 0 to 360 degree range.
 *
 * Computes the base inward radial angle from the specified (x, y) grid index to 
 * the center of the array. It applies the provided dynamic offset (theta) based 
 * on which motor is being calculated, and normalizes the final result to a 
 * standard Cartesian angle (0 to 360 degrees, counterclockwise from the positive x-axis).
 *
 * @param x             The horizontal index in the grid (e.g., 1 to 8).
 * @param y             The vertical index in the grid (e.g., 1 to 3).
 * @param theta         The dynamic offset angle calculated for this coordinate's V-shape.
 * @param is_hour_hand  True to calculate the hour hand (adds theta), 
 * False to calculate the minute hand (subtracts theta).
 * @return float        The absolute target angle in degrees, strictly bounded [0.0, 360.0).
 */
float calculateAbsoluteMotorAngle(float x, float y, float theta, bool is_hour_hand) {
  const float centerX = 4.5f;
  const float centerY = 2.0f;

  // atan2f uses (y, x). Multiply by 180/PI to convert to degrees.
  float base_angle = atan2f(centerY - y, centerX - x) * 57.29578f;

  float adjusted_angle = is_hour_hand ? base_angle + theta : base_angle - theta;

  // Mathematically robust normalization to strict [0, 360) range AND CW inversion
  float normalized_angle = fmodf(adjusted_angle, 360.0f);
  if (normalized_angle < 0) {
      normalized_angle += 360.0f;
  }

  // Invert CCW to CW for the physical motors
  return fmodf((360.0f - normalized_angle), 360.0f);
}

void set_circle()
{
  const int SETUP_WAIT_MS = 10000;   // ms: reach attractor field + pause
  const float DELAY_SPEED = 300.0f; // ms: per unit grid distance (ripple speed)
  const int PHASE1_SPEED = 500;     // motor speed for phase 1
  const int PHASE1_ACCEL = 150;     // motor acceleration for phase 1
  const int PHASE2_SPEED = 400;     // motor speed for phase 2
  const int PHASE2_ACCEL = 100;     // motor acceleration for phase 2
  const float INNER_BOUND = 70.0f;  // degrees: minimum angle offset at center (controls V-shape pointiness)
  const float OUTER_BOUND = 85.0f;  // degrees: maximum angle offset at edges (values near 90 create circular tangent)

  // Phase 1: All hands move to angles that form a circle pattern
  uint16_t angles_phase1[24][2]; //24 clock with each 2 hands: stores individual angles
  for (int i = 0; i < 24; i++) {
    int c = i / 3; // column (0 to 7)
    int r = i % 3; // row (0 to 2)

    // Calculate dynamic theta for this clock's position to create the attractor field illusion
    float theta = calculateAttractorAngle(c + 1, r + 1, INNER_BOUND, OUTER_BOUND);

    // Calculate distinct absolute angles for both hands
    float target_hour = calculateAbsoluteMotorAngle(c + 1.0f, r + 1.0f, theta, true);
    float target_minute = calculateAbsoluteMotorAngle(c + 1.0f, r + 1.0f, theta, false);

    // Round floats to nearest integer and cast to uint16_t
    angles_phase1[i][0] = static_cast<uint16_t>(std::round(target_minute));
    angles_phase1[i][1] = static_cast<uint16_t>(std::round(target_hour));
  }

  set_custom_clock(angles_phase1, PHASE1_SPEED, PHASE1_ACCEL, MIN_DISTANCE);
  _delay(SETUP_WAIT_MS); // Wait for setup

  // Phase 2 + 3: Ripple spin to final digits
  set_speed(PHASE2_SPEED);
  set_acceleration(PHASE2_ACCEL);

  t_full_clock final_clock = get_clock_state_from_time(last_hour, last_minute);

  // Sort the 24 clocks by delay
  struct ClockTask {
    int c; //column
    int r; //row
    int delay_ms;
  } tasks[24];

  for (int i = 0; i < 24; i++) {
    tasks[i].c = i / 3;
    tasks[i].r = i % 3;
    tasks[i].delay_ms = calculate_start_delay(tasks[i].c + 1, tasks[i].r + 1, DELAY_SPEED);
  }

  // Bubble sort tasks by delay
  for (int i = 0; i < 23; i++) {
    for (int j = 0; j < 23 - i; j++) {
      if (tasks[j].delay_ms > tasks[j+1].delay_ms) {
        ClockTask temp = tasks[j];
        tasks[j] = tasks[j+1];
        tasks[j+1] = temp;
      }
    }
  }

  unsigned long start_time = millis();
  for (int i = 0; i < 24; i++) {
    int c = tasks[i].c;
    int r = tasks[i].r;

    // Wait until it's time to trigger this clock
    while (millis() - start_time < (unsigned long)tasks[i].delay_ms) {
       update_MDNS();
       handle_webclient();
       if (is_ota_in_progress()) return;
       delay(5);
    }

    int board = c;
    int clock_idx = r;

    // Get target angle from final_clock
    int digit_idx = board / 2;
    int half_idx = board % 2;

    int target_h = final_clock.digit[digit_idx].halfs[half_idx].clocks[clock_idx].angle_h;
    int target_m = final_clock.digit[digit_idx].halfs[half_idx].clocks[clock_idx].angle_m;

    // Spin hands in opposite directions: Hour hand CW, Minute hand CCW
    set_single_clock_target_dir(board, clock_idx, target_h, target_m, CLOCKWISE2, COUNTERCLOCKWISE2);
  }
}


/**
 * Returns the angle (degrees, 0 = right, CW+) for a hand at grid
 * position (x, y) pointing toward the attractor at (cx, cy).
 */
float get_angle_to_attractor(int x, int y, float cx, float cy)
{
  return atan2(cy - y, cx - x) * 180.0f / M_PI;
}


void set_spiral()
{
  const int SETUP_WAIT_MS = 10000;   // ms: reach attractor field + pause
  const float DELAY_SPEED = 300.0f; // ms: per unit grid distance (ripple speed)
  const int PHASE1_SPEED = 500;     // motor speed for phase 1
  const int PHASE1_ACCEL = 150;     // motor acceleration for phase 1
  const int PHASE2_SPEED = 400;     // motor speed for phase 2
  const int PHASE2_ACCEL = 100;     // motor acceleration for phase 2

  // Phase 1: All hands move to attractor field (4.5, 2.0)

  uint16_t angles_phase1[24][2]; //24 clock with each 2 hands: stores individual angles
  for (int i = 0; i < 24; i++) {
    int c = i / 3; // column (0 to 7)
    int r = i % 3; // row (0 to 2)

    float angle = get_angle_to_attractor(c + 1, r + 1, 4.5f, 2.0f);

    // Ensure angle is in [0, 360) and positive
    int target = (int)round(360.0f - angle) % 360;
    if (target < 0) target += 360;
    angles_phase1[i][0] = target;
    angles_phase1[i][1] = target;
  }
  set_custom_clock(angles_phase1, PHASE1_SPEED, PHASE1_ACCEL, MIN_DISTANCE);
  _delay(SETUP_WAIT_MS); // Wait for setup

  // Phase 2 + 3: Ripple spin to final digits
  set_speed(PHASE2_SPEED);
  set_acceleration(PHASE2_ACCEL);
  set_direction(CLOCKWISE2);

  t_full_clock final_clock = get_clock_state_from_time(last_hour, last_minute);

  // Sort the 24 clocks by delay
  struct ClockTask {
    int c; //column
    int r; //row
    int delay_ms;
  } tasks[24];

  for (int i = 0; i < 24; i++) {
    tasks[i].c = i / 3;
    tasks[i].r = i % 3;
    tasks[i].delay_ms = calculate_start_delay(tasks[i].c + 1, tasks[i].r + 1, DELAY_SPEED);
  }

  // Bubble sort tasks by delay
  for (int i = 0; i < 23; i++) {
    for (int j = 0; j < 23 - i; j++) {
      if (tasks[j].delay_ms > tasks[j+1].delay_ms) {
        ClockTask temp = tasks[j];
        tasks[j] = tasks[j+1];
        tasks[j+1] = temp;
      }
    }
  }

  unsigned long start_time = millis();
  for (int i = 0; i < 24; i++) {
    int c = tasks[i].c;
    int r = tasks[i].r;

    // Wait until it's time to trigger this clock
    while (millis() - start_time < (unsigned long)tasks[i].delay_ms) {
       update_MDNS();
       handle_webclient();
       if (is_ota_in_progress()) return;
       delay(5);
    }

    int board = c;
    int clock_idx = r;

    // Get target angle from final_clock
    int digit_idx = board / 2;
    int half_idx = board % 2;

    int target_h = final_clock.digit[digit_idx].halfs[half_idx].clocks[clock_idx].angle_h;
    int target_m = final_clock.digit[digit_idx].halfs[half_idx].clocks[clock_idx].angle_m;

    set_single_clock_target(board, clock_idx, target_h, target_m);
  }
}

void set_attract()
{
  const int SETUP_WAIT_MS = 3500;  // ms: hands reach initial position
  const float TRAVEL_SPD = 1.0f;   // grid units per second
  const int UPDATE_MS = 150;       // ms between position updates (slower for physical stability)
  const int TRACKING_SPEED = 800;  // high speed for quick tracking adjustments
  const int TRACKING_ACCEL = 500;  
  
  uint16_t angles[24][2];

  // Phase 1: Align to initial attractor position (0,0)
  for (int i = 0; i < 24; i++) {
    int c = i / 3;
    int r = i % 3;
    float angle = get_angle_to_attractor(c + 1, r + 1, 0.0f, 0.0f);
    int target = (int)round(360.0f - angle) % 360;
    if (target < 0) target += 360;
    angles[i][0] = target;
    angles[i][1] = target;
  }
  set_custom_clock(angles, 400, 150, MIN_DISTANCE);
  _delay(SETUP_WAIT_MS);

  // Phase 2: Follow rectangle path (0,0) -> (9,0) -> (9,4) -> (0,4) -> (0,0)
  const float waypoints[5][2] = {
    {0.0f, 0.0f},
    {9.0f, 0.0f},
    {9.0f, 4.0f},
    {0.0f, 4.0f},
    {0.0f, 0.0f}
  };

  for (int i = 0; i < 4; i++) {
    float x0 = waypoints[i][0];
    float y0 = waypoints[i][1];
    float x1 = waypoints[i+1][0];
    float y1 = waypoints[i+1][1];
    
    float dx = x1 - x0;
    float dy = y1 - y0;
    float dist = sqrt(dx*dx + dy*dy);
    
    int steps = ceil((dist / TRAVEL_SPD) * 1000.0f / UPDATE_MS);

    for (int s = 0; s < steps; s++) {
      float t = (float)s / steps;
      float cx = x0 + dx * t;
      float cy = y0 + dy * t;

      for (int c_idx = 0; c_idx < 24; c_idx++) {
        int c = c_idx / 3;
        int r = c_idx % 3;
        float angle = get_angle_to_attractor(c + 1, r + 1, cx, cy);
        int target = (int)round(360.0f - angle) % 360;
        if (target < 0) target += 360;
        angles[c_idx][0] = target;
        angles[c_idx][1] = target;
      }

      unsigned long step_start = millis();
      set_custom_clock(angles, TRACKING_SPEED, TRACKING_ACCEL, MIN_DISTANCE);
      
      while (millis() - step_start < (unsigned long)UPDATE_MS) {
        update_MDNS();
        handle_webclient();
        if (is_ota_in_progress()) return;
        delay(5);
      }
    }
  }

  // Phase 3: Transition to final time
  set_speed(400);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
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