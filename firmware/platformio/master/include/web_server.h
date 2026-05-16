#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WebServer.h>

#include "web_page.h"
#include "clock_manager.h"
#include "clock_config.h"

typedef struct browser_time
{
  int hour;
  int minute;
  int second;
  int day;
  int month;
  int year;
} t_browser_time;

/**
 * Starts and configures the server
*/
void server_start();

/**
 * Client handling, needs to be called on the main loop
*/
void handle_webclient();

/**
 * Stops the server and free resources
*/
void server_stop();

/**
 * Handles GET /
*/
void handle_get();

/**
 * Handles GET /config
*/
void handle_get_config();

/**
 * Handles POST /time
*/
void handle_post_time();

/**
 * Handles POST /adjust
*/
void handle_post_adjust();

/**
 * Handles POST /mode
*/
void handle_post_mode();

/**
 * Handles POST /sleep
*/
void handle_post_sleep();

/**
 * Handles POST /connection
*/
void handle_post_connection();

/**
 * Handles POST /test
*/
void handle_post_test();

/**
 * Check if an OTA update is currently in progress
 * @return true while an OTA firmware upload is being processed
*/
bool is_ota_in_progress();

/**
 * Handles GET /update — serves the OTA firmware upload page (EXT_CONN mode only)
*/
void handle_ota_page();

/**
 * Handles POST /update response — sends result and reboots on success
*/
void handle_ota_complete();

/**
 * Handles POST /update file upload stream (called per-chunk by WebServer)
*/
void handle_ota_upload();

/**
 * Check if the client changed the time
 * @return true if the client makes a request, false otherwise
*/
bool is_time_changed_browser();

/**
 * Return the client's browser time
 * @return time
*/
t_browser_time get_browser_time();

#endif