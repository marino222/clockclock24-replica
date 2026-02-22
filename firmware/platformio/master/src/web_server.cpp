#include "web_server.h"

WebServer _server(80);

t_browser_time _browser_time = {0, 0, 0, 0, 0, 0};
bool _time_changed_browser = false;

void server_start()
{
  // Setup web server connection
  _server.enableCORS(true);
  _server.begin();
  _server.on("/", HTTP_GET, handle_get);
  _server.on("/config", HTTP_GET, handle_get_config);
  _server.on("/time", HTTP_POST, handle_post_time);
  _server.on("/adjust", HTTP_POST, handle_post_adjust);
  _server.on("/mode", HTTP_POST, handle_post_mode);
  _server.on("/sleep", HTTP_POST, handle_post_sleep);
  _server.on("/connection", HTTP_POST, handle_post_connection);
  _server.on("/test", HTTP_POST, handle_post_test);
  Serial.println("WebServer setup done");
}

void handle_webclient()
{
  _server.handleClient();
}

void server_stop()
{
  _server.close();
}

void handle_get()
{
  Serial.println("Handle GET /");
  _server.send(200, "text/html", WEB_PAGE);
}

void handle_get_config()
{
  Serial.println("Handle GET /config");\
  char payload[1024];
  {
    char s_time[512] = "[";
    for (int i = 0; i < 7; i++)
    {
      strncat(s_time, "[", 2);
      for (int j = 0; j < 24; j++)
      {
        strncat(s_time, get_sleep_time(i, j) ? "1" : "0", 2);
        if(j < 23)
          strncat(s_time,"," , sizeof(2));
      }
      strncat(s_time, "]", sizeof(2));
      if(i < 6)
        strncat(s_time,"," , sizeof(2));
    }
    strncat(s_time, "]", sizeof(2));
    snprintf(payload, sizeof(payload), 
      "{\"clock_mode\":%d,"
      "\"wireless_mode\":%d,"
      "\"ssid\":\"%s\","
      "\"password\":\"%s\","
      "\"sleep_time\":%s}",
      get_clock_mode(), get_connection_mode(), get_ssid(), get_password(), s_time);
  }
  _server.send(200, "application/json", payload);
}

void handle_post_time()
{
  Serial.println("Handle POST /time");
  if (_server.hasArg("h"))
    _browser_time.hour = _server.arg("h").toInt();
  if (_server.hasArg("m"))
    _browser_time.minute = _server.arg("m").toInt();
  if (_server.hasArg("s"))
    _browser_time.second = _server.arg("s").toInt();
  if (_server.hasArg("D"))
    _browser_time.day = _server.arg("D").toInt();
  if (_server.hasArg("M"))
    _browser_time.month = _server.arg("M").toInt();
  if (_server.hasArg("Y"))
    _browser_time.year = _server.arg("Y").toInt();
  if (_server.hasArg("timezone"))
  {
    int _browser_timezone = _server.arg("timezone").toInt();
    set_timezone(_browser_timezone);
  }
  _time_changed_browser = true;
  _server.send(200, "text/plain", "");
  Serial.printf("Time received: %d:%d:%d\n", 
    _browser_time.hour, _browser_time.minute, _browser_time.second);
}

void handle_post_adjust()
{
  Serial.println("Handle POST /adjust");
  int clock_index = 0;
  int m_amount = 0;
  int h_amount = 0;
  if (_server.hasArg("index"))
    clock_index = _server.arg("index").toInt();
  if (_server.hasArg("m_amount"))
    m_amount = _server.arg("m_amount").toInt();
  if (_server.hasArg("h_amount"))
    h_amount = _server.arg("h_amount").toInt();

  _server.send(200, "text/plain", "");

  Serial.printf("Adjust received, clock: %d, m_amount: %d, h_amount: %d\n", 
    clock_index, m_amount, h_amount);
  adjust_hands(clock_index, m_amount, h_amount);
}

void handle_post_mode()
{
  Serial.println("Handle POST /mode");
  if (_server.hasArg("mode"))
    set_clock_mode(_server.arg("mode").toInt());
  _server.send(200, "text/plain", "");
}

void handle_post_sleep()
{
  Serial.println("Handle POST /sleep");
  if (_server.hasArg("day"))
  {
    int day = _server.arg("day").toInt();
    for(int i = 0; i < 24; i++)
    {
      char arg[8];
      snprintf(arg, sizeof(arg), "h%d", i);
      if (_server.hasArg(arg))
        set_sleep_time(day, i, _server.arg(arg).toInt() == 0 ? false : true);
    }
    save_sleep_time();
  }
  _server.send(200, "text/html", "");
}

void handle_post_connection()
{
  Serial.println("Handle POST /connection");
  if (_server.hasArg("mode"))
    set_connection_mode(_server.arg("mode").toInt());
  if (_server.hasArg("ssid"))
    set_ssid(_server.arg("ssid").c_str());
  if (_server.hasArg("password"))
    set_password(_server.arg("password").c_str());
  _server.send(200, "text/plain", "");
  end_config();
  ESP.restart();
}

bool is_time_changed_browser()
{
  bool tmp = _time_changed_browser;
  _time_changed_browser = false;
  return tmp;
}

t_browser_time get_browser_time()
{
  return _browser_time;
}

void handle_post_test()
{
  Serial.println("Handle POST /test");
  
  // Parameters:
  // board: 0-7 (half-digit board index)
  // clock: 0-2 (clock index on the board)
  // angle_h: 0-359 (hour hand angle, optional)
  // angle_m: 0-359 (minute hand angle, optional)
  // speed: motor speed (optional, default uses global setting)
  // accel: motor acceleration (optional, default uses global setting)
  
  if (!_server.hasArg("board") || !_server.hasArg("clock"))
  {
    _server.send(400, "text/plain", "Missing required parameters: board, clock");
    return;
  }
  
  int board = _server.arg("board").toInt();
  int clock = _server.arg("clock").toInt();
  
  // Validate parameters
  if (board < 0 || board > 7)
  {
    _server.send(400, "text/plain", "board must be 0-7");
    return;
  }
  
  if (clock < 0 || clock > 2)
  {
    _server.send(400, "text/plain", "clock must be 0-2");
    return;
  }
  
  // Get current state or create new one
  t_half_digitl lite_half = {0};
  
  // Set angles if provided
  if (_server.hasArg("angle_h"))
  {
    int angle_h = _server.arg("angle_h").toInt();
    lite_half.clocks[clock].angle_h = angle_h % 360;
  }
  
  if (_server.hasArg("angle_m"))
  {
    int angle_m = _server.arg("angle_m").toInt();
    lite_half.clocks[clock].angle_m = angle_m % 360;
  }
  
  // Convert to full half digit
  t_half_digit full_half = get_full_half_digit(lite_half);
  
  // Override speed/accel if provided
  if (_server.hasArg("speed"))
  {
    int speed = _server.arg("speed").toInt();
    full_half.clocks[clock].speed_h = speed;
    full_half.clocks[clock].speed_m = speed;
  }
  
  if (_server.hasArg("accel"))
  {
    int accel = _server.arg("accel").toInt();
    full_half.clocks[clock].accel_h = accel;
    full_half.clocks[clock].accel_m = accel;
  }
  
  // Send command
  send_half_digit(board, full_half);
  
  _server.send(200, "text/plain", "OK");
  
  Serial.printf("Test motor - Board: %d, Clock: %d, Angle H: %d, Angle M: %d\n",
    board, clock, full_half.clocks[clock].angle_h, full_half.clocks[clock].angle_m);
}