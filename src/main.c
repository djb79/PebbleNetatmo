#include <pebble.h>
#include <math.h>
#include "weather.h"
#include "clock_digit.h"  
  
#define KEY_TEMPERATURE_OUT 0
#define KEY_TEMPERATURE_UP 1
#define KEY_TEMPERATURE_MID 2
#define KEY_TEMPERATURE_DOWN 3
#define KEY_CO2_UP 4
#define KEY_CO2_MID 5
#define KEY_CO2_DOWN 6
#define KEY_CONDITION 7
  
// windows and layers
static Window *s_main_window;
static Layer* sidebarLayer;

static TextLayer *temperature_layer_out;
static TextLayer *temperature_layer_up;
static TextLayer *temperature_layer_mid;
static TextLayer *temperature_layer_down;

// images
//static GDrawCommandImage* disconnectImage;
static GDrawCommandImage* batteryImage;

//static bool isPhoneConnected;

// fonts
static GFont temperatureFont;

// the four digits on the clock, ordered h1 h2, m1 m2
static ClockDigit clockDigits[4];

// color for co2 value
GColor co2_color(int value) {
  if(value < 650) {
    return GColorIslamicGreen;
  }  
  if(value >= 650 && value < 850) {
    return GColorSpringBud;
  }
  if(value >= 850 && value < 1250) {
    return GColorYellow;
  }
  if(value >= 1250 && value < 1500) {
    return GColorChromeYellow;
  }  
  if(value >= 1500 && value < 1750) {
    return GColorOrange;
  }
  else {
    return GColorRed;
  }
}

void update_clock() {
  // Get a tm structure
  time_t rawTime;
  struct tm* timeInfo;

  time(&rawTime);
  timeInfo = localtime(&rawTime);

  int hour = timeInfo->tm_hour;

  // use the blank image for the leading hour digit if needed
  if(hour / 10 != 0) {
    ClockDigit_setNumber(&clockDigits[0], hour / 10);
  } else {
    ClockDigit_setBlank(&clockDigits[0]);
  }
  
  ClockDigit_setNumber(&clockDigits[1], hour % 10);
  ClockDigit_setNumber(&clockDigits[2], timeInfo->tm_min  / 10);
  ClockDigit_setNumber(&clockDigits[3], timeInfo->tm_min  % 10);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Clock updated!");
  
}

void redrawSidebar() {
  layer_mark_dirty(sidebarLayer);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Sidebar redrawed!");
}

// forces everything on screen to be redrawn -- perfect for keeping track of settings!
void forceScreenRedraw() {
  // maybe the colors changed!
  for(int i = 0; i < 4; i++) {
    ClockDigit_setColor(&clockDigits[i], GColorIslamicGreen, GColorClear);
  }

  // maybe the language changed!
  update_clock();
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Screen redrawed!");
}

void drawBatteryStatus(GContext* ctx) {
  BatteryChargeState chargeState = battery_state_service_peek();

  int batteryPositionY = 72;
  int width = roundf(18 * chargeState.charge_percent / 100.0f);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery charge state is %d", chargeState.charge_percent);
  
  graphics_context_set_fill_color(ctx, GColorBlack);

  if(chargeState.charge_percent <= 20) {
    graphics_context_set_fill_color(ctx, GColorRed);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery under 20 percent!");
  }

  if(chargeState.charge_percent <= 50) {
    gdraw_command_image_draw(ctx, batteryImage, GPoint(3, batteryPositionY));
    graphics_fill_rect(ctx, GRect(6, 8 + batteryPositionY, width, 8), 0, GCornerNone);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery under 50 percent!");
  }
}

void sidebarLayerUpdateProc(Layer *l, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_rect(ctx, layer_get_bounds(l), 0, GCornerNone);

  GDrawCommandImage* Weather_currentWeatherIcon;
  Weather_currentWeatherIcon = gdraw_command_image_create_with_resource(RESOURCE_ID_WEATHER_PARTLY_CLOUDY);
  gdraw_command_image_draw(ctx, Weather_currentWeatherIcon, GPoint(3, 4));

  //if (!isPhoneConnected) {
  //  gdraw_command_image_draw(ctx, disconnectImage, GPoint(3, 60));
  //}
  //if(isPhoneConnected) {
    drawBatteryStatus(ctx);
  //}
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Sidebar updatet!");
}

static void main_window_load(Window *window) {
  window_set_background_color(s_main_window, GColorBlack);

  ClockDigit_construct(&clockDigits[0], GPoint(7, 7));
  ClockDigit_construct(&clockDigits[1], GPoint(60, 7));
  ClockDigit_construct(&clockDigits[2], GPoint(7, 90));
  ClockDigit_construct(&clockDigits[3], GPoint(60, 90));

  for(int i = 0; i < 4; i++) {
    ClockDigit_setColor(&clockDigits[i], GColorIslamicGreen, GColorClear);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(clockDigits[i].imageLayer));
  }
  
  // load fonts
  temperatureFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  
  // load Images
  batteryImage = gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_BG);
  //disconnectImage = gdraw_command_image_create_with_resource(RESOURCE_ID_DISCONNECTED);

  // Create Sidebar
  sidebarLayer = layer_create(GRect(114, 0, 30, 96));
  layer_add_child(window_get_root_layer(window), sidebarLayer);
  layer_set_update_proc(sidebarLayer, sidebarLayerUpdateProc);  
 
  // Create temperature Layer (Out)
  temperature_layer_out = text_layer_create(GRect(114, 30, 30, 24));
  text_layer_set_background_color(temperature_layer_out, GColorClear);
  text_layer_set_text_color(temperature_layer_out, GColorBlack);
  text_layer_set_text_alignment(temperature_layer_out, GTextAlignmentCenter);
  text_layer_set_font(temperature_layer_out, temperatureFont);
  text_layer_set_text(temperature_layer_out, "-");

  // Create temperature Layer (Up)
  temperature_layer_up = text_layer_create(GRect(114, 96, 30, 24));
  text_layer_set_background_color(temperature_layer_up, GColorDarkGray);
  text_layer_set_text_color(temperature_layer_up, GColorBlack);
  text_layer_set_text_alignment(temperature_layer_up, GTextAlignmentCenter);
  text_layer_set_font(temperature_layer_up, temperatureFont);
  text_layer_set_text(temperature_layer_up, "-");
  
  // Create temperature Layer (Mid)
  temperature_layer_mid = text_layer_create(GRect(114, 120, 30, 24));
  text_layer_set_background_color(temperature_layer_mid, GColorDarkGray);
  text_layer_set_text_color(temperature_layer_mid, GColorBlack);
  text_layer_set_text_alignment(temperature_layer_mid, GTextAlignmentCenter);
  text_layer_set_font(temperature_layer_mid, temperatureFont);
  text_layer_set_text(temperature_layer_mid, "-");

  // Create temperature Layer (Down)
  temperature_layer_down = text_layer_create(GRect(114, 144, 30, 24));
  text_layer_set_background_color(temperature_layer_down, GColorDarkGray);
  text_layer_set_text_color(temperature_layer_down, GColorBlack);
  text_layer_set_text_alignment(temperature_layer_down, GTextAlignmentCenter);
  text_layer_set_font(temperature_layer_down, temperatureFont);
  text_layer_set_text(temperature_layer_down, "-");
  
  // Create child layers
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(temperature_layer_out));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(temperature_layer_up));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(temperature_layer_mid));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(temperature_layer_down));

  // Make sure the time is displayed from the start
  forceScreenRedraw();
  update_clock();
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window loaded!");
}

static void main_window_unload(Window *window) {
  for(int i = 0; i < 4; i++) {
    ClockDigit_destruct(&clockDigits[i]);
  }
  
  // Destroy Sidebar
  layer_destroy(sidebarLayer);
  
  // Unload GFont
  fonts_unload_custom_font(temperatureFont);
  
  // Destroy weather elements
  text_layer_destroy(temperature_layer_out);
  text_layer_destroy(temperature_layer_up);
  text_layer_destroy(temperature_layer_mid);
  text_layer_destroy(temperature_layer_down);
  
  // Destroy Icons
  gdraw_command_image_destroy(batteryImage);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window unloaded!");
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Get weather update every 15 minutes
  if(tick_time->tm_min % 15 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
  
  update_clock();
}

void batteryStateChanged(BatteryChargeState charge_state) {
  redrawSidebar();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery state has changed!");
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperatureOut[8];
  static char temperatureUp[8];
  static char temperatureMid[8];
  static char temperatureDown[8];
  static int co2Up;
  static int co2Mid;
  static int co2Down;
  static int condition;
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE_OUT:
      snprintf(temperatureOut, sizeof(temperatureOut), "%d°", (int)t->value->int32);
      break;
    case KEY_TEMPERATURE_UP:
      snprintf(temperatureUp, sizeof(temperatureUp), "%d°", (int)t->value->int32);
      break;
    case KEY_TEMPERATURE_MID:
      snprintf(temperatureMid, sizeof(temperatureMid), "%d°", (int)t->value->int32);
      break;
    case KEY_TEMPERATURE_DOWN:
      snprintf(temperatureDown, sizeof(temperatureDown), "%d°", (int)t->value->int32);
      break;
    case KEY_CO2_UP:
      co2Up = (int)t->value->int32;
      break;
    case KEY_CO2_MID:
      co2Mid = (int)t->value->int32;
      break;
    case KEY_CO2_DOWN:
      co2Down = (int)t->value->int32;
      break;
    case KEY_CONDITION:
      condition = (int)t->value->int32;
      Weather_setCondition(condition);      
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  text_layer_set_text(temperature_layer_out, temperatureOut);
  text_layer_set_text(temperature_layer_up, temperatureUp);
  text_layer_set_text(temperature_layer_mid, temperatureMid);
  text_layer_set_text(temperature_layer_down, temperatureDown);

  // Color for CO2 values
  text_layer_set_background_color(temperature_layer_up, co2_color(co2Up));
  text_layer_set_background_color(temperature_layer_mid, co2_color(co2Mid));
  text_layer_set_background_color(temperature_layer_down, co2_color(co2Down));

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message recieved!");  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! %d %d %d", reason, APP_MSG_SEND_TIMEOUT, APP_MSG_SEND_REJECTED);
}


static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
  
static void init() {
  Weather_init();
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Watch messaging is started!");
  
  // register with battery service
  battery_state_service_subscribe(batteryStateChanged);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  Weather_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
