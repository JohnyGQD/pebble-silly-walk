#include <pebble.h>

static Window *window;

static BitmapLayer *background_layer;
static Layer *info_container, *info_layer, *digital_container;
static TextLayer *bluetooth_layer, *battery_layer, *day_layer, *date_layer, *digital_layer;
static RotBitmapLayer *black_hour_layer, *white_hour_layer, *black_minute_layer, *white_minute_layer, *black_second_layer, *white_second_layer;

static char date_text[] = "Xxxxxxxxxxxxxxxxxxxxxxx 00", day_text[] = "Xxxxxxxxxxxxxxxxxxxxxxx 00", battery_text[] = "Xxxxxxxxxxxxxxxxxxxxxxx 00", digital_text[] = "Xxxxxxxxxxxxxxxxxxxxxxx 00";

static GBitmap *background_bitmap, *background_simple_bitmap;
static GBitmap *black_hour_bitmap, *white_hour_bitmap, *black_minute_bitmap, *white_minute_bitmap, *black_second_bitmap, *white_second_bitmap;

enum Settings { setting_seconds = 1, setting_background, setting_info, setting_digital };
static enum SettingSeconds { seconds_on = 0, seconds_off } seconds;
static enum SettingBackground { background_rich = 0, background_simple } background;
static enum SettingInfo { info_on = 0, info_off } info;
static enum SettingDigital { digital_off = 0, digital_24, digital_12 } digital;
static AppSync app;
static uint8_t buffer[256];

int32_t last_hour = -1, last_minute = -1;

void update_time(struct tm* t) {
  // Find out what needs to be updated
  bool hour_changed = false;
  bool minute_changed = false;
  
  if (last_hour != t->tm_hour) {
    hour_changed = true;
    last_hour = t->tm_hour;
  }
  
  if (last_minute != t->tm_min) {
    minute_changed = true;
    last_minute = t->tm_min;
  }
  
  // Update hour hand
  if (hour_changed) {
    int32_t hour_angle = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 30) + (t->tm_min/2)) / 360;
    GRect hour_frame = layer_get_frame((Layer*) white_hour_layer);
    hour_frame.origin.x = (144/2) - (hour_frame.size.w/2);  
    hour_frame.origin.y = (168/2) - (hour_frame.size.h/2);
  
    rot_bitmap_layer_set_angle(white_hour_layer, hour_angle);
    layer_set_frame((Layer*) white_hour_layer, hour_frame);
  	layer_mark_dirty((Layer*) white_hour_layer);
    
    rot_bitmap_layer_set_angle(black_hour_layer, hour_angle);
    layer_set_frame((Layer*) black_hour_layer, hour_frame);
  	layer_mark_dirty((Layer*) black_hour_layer);
  }
  
  // Update minute hand
  if (minute_changed) {
    int32_t minute_angle = TRIG_MAX_ANGLE * (t->tm_min * 6) / 360;
    GRect minute_frame = layer_get_frame((Layer*) white_minute_layer);
    minute_frame.origin.x = (144/2) - (minute_frame.size.w/2);  
    minute_frame.origin.y = (168/2) - (minute_frame.size.h/2);
    
    rot_bitmap_layer_set_angle(white_minute_layer, minute_angle);
    layer_set_frame((Layer*) white_minute_layer, minute_frame);
  	layer_mark_dirty((Layer*) white_minute_layer);
    
    rot_bitmap_layer_set_angle(black_minute_layer, minute_angle);
    layer_set_frame((Layer*) black_minute_layer, minute_frame);
  	layer_mark_dirty((Layer*) black_minute_layer);
  }
  
  // Update second hand
  if (seconds == seconds_on) {
    int32_t second_angle = TRIG_MAX_ANGLE * (t->tm_sec * 6) / 360;
    GRect second_frame = layer_get_frame((Layer*) white_second_layer);
    second_frame.origin.x = (144/2) - (second_frame.size.w/2);  
    second_frame.origin.y = (168/2) - (second_frame.size.h/2);
    
    rot_bitmap_layer_set_angle(white_second_layer, second_angle);
    layer_set_frame((Layer*) white_second_layer, second_frame);
  	layer_mark_dirty((Layer*) white_second_layer);
    
    rot_bitmap_layer_set_angle(black_second_layer, second_angle);
    layer_set_frame((Layer*) black_second_layer, second_frame);
  	layer_mark_dirty((Layer*) black_second_layer);
  }
  
  // Update info layer, but not that often
  if ((info == info_on) && (minute_changed)) {    
    if (bluetooth_connection_service_peek())
      text_layer_set_text(bluetooth_layer, "Linked");
    else      
      text_layer_set_text(bluetooth_layer, "Offline");
    
    BatteryChargeState initial = battery_state_service_peek();
    bool battery_plugged = initial.is_plugged;
    uint8_t battery_level = initial.charge_percent;
    
    if (battery_plugged)
	    snprintf(battery_text, sizeof(battery_text), "! %d%%", battery_level);
    else 
	    snprintf(battery_text, sizeof(battery_text), "%d%%", battery_level);
      
    text_layer_set_text(battery_layer, battery_text);
    
    strftime(day_text, sizeof(day_text), "%a", t);
    text_layer_set_text(day_layer, day_text);
    
    strftime(date_text, sizeof(date_text), "%b %e", t);
    text_layer_set_text(date_layer, date_text);
  }
  
  // Update digital time
  if ((digital != digital_off) && (minute_changed)) {
    if (digital == digital_12)
      strftime(digital_text, sizeof(digital_text), "%I:%M %p", t);
    else
      strftime(digital_text, sizeof(digital_text), "%H:%M", t);
    
    text_layer_set_text(digital_layer, digital_text);
  }
}

void update_now() {
  last_hour = -1;
  last_minute = -1;
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);
}

void handle_tick(struct tm *tick_time, TimeUnits units_changed) {  
  update_time(tick_time);
}

void enable_seconds() {  
  Layer *window_layer = window_get_root_layer(window);
  layer_add_child(window_layer, bitmap_layer_get_layer((BitmapLayer*) white_second_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer((BitmapLayer*) black_second_layer));
  
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(SECOND_UNIT, &handle_tick);
}

void disable_seconds() {
  layer_remove_from_parent(bitmap_layer_get_layer((BitmapLayer*) white_second_layer));
  layer_remove_from_parent(bitmap_layer_get_layer((BitmapLayer*) black_second_layer));
  
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_tick);
}

void enable_info() {
  layer_add_child(info_container, info_layer);  
	layer_mark_dirty(info_container);
}

void disable_info() {
  layer_remove_from_parent(info_layer);  
	layer_mark_dirty(info_container);
}

void enable_digital() {
  layer_add_child(digital_container, text_layer_get_layer(digital_layer));  
	layer_mark_dirty(digital_container);
  
  update_now();
}

void disable_digital() {
  layer_remove_from_parent(text_layer_get_layer(digital_layer));  
	layer_mark_dirty(digital_container);
}

static void tuple_changed_callback(const uint32_t key, const Tuple* tuple_new, const Tuple* tuple_old, void* context) {
  int value = tuple_new->value->uint8;
  switch (key) {
    case setting_seconds:
      seconds = value;
      persist_write_int(setting_seconds, seconds);
      
      if (seconds == seconds_on)
        enable_seconds();
      else
        disable_seconds();
      break;
    case setting_background:
      background = value;
      persist_write_int(setting_background, background);
      
      bitmap_layer_set_bitmap(background_layer, (background == background_simple) ? background_simple_bitmap : background_bitmap);
      layer_mark_dirty((Layer*) background_layer);
      break;
    case setting_info:
      info = value;
      persist_write_int(setting_info, info);
      
      if (info == info_on)
        enable_info();
      else
        disable_info();
      break;
    case setting_digital:
      digital = value;
      persist_write_int(setting_digital, digital);
      
      if (digital == digital_off)
        disable_digital();
      else
        enable_digital();
      break;
  }
}

static void app_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void* context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "app error %d", app_message_error);
}

void init(void) {
  // Init the global stuff
	window = window_create();
	window_stack_push(window, true);
  
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  
  last_hour = -1;
  last_minute = -1;
  
  // Get local values before the JS is ready to prevent flickering
  seconds = persist_read_int(setting_seconds);
  background = persist_read_int(setting_background);
  info = persist_read_int(setting_info);
  digital = persist_read_int(setting_digital);
  
  // Load settings 
  Tuplet tuples[] = {
    TupletInteger(setting_seconds, seconds),
    TupletInteger(setting_background, background),
    TupletInteger(setting_info, info),
    TupletInteger(setting_digital, digital)
  };
  app_message_open(160, 160);
  app_sync_init(&app, buffer, sizeof(buffer), tuples, ARRAY_LENGTH(tuples),
                tuple_changed_callback, app_error_callback, NULL);
  
  // Init background
  background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);  
  background_simple_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_SIMPLE);
  background_layer = bitmap_layer_create(window_bounds);
  bitmap_layer_set_bitmap(background_layer, (background == background_simple) ? background_simple_bitmap : background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
  
  // Init info
  info_container = layer_create(window_bounds);  
  layer_add_child(window_layer, info_container);
  
  info_layer = layer_create(window_bounds);
  
  bluetooth_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { 44, 18 } });
  text_layer_set_font(bluetooth_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(bluetooth_layer, GTextAlignmentCenter);
  text_layer_set_background_color(bluetooth_layer, GColorBlack);
  text_layer_set_text_color(bluetooth_layer, GColorWhite);
  layer_add_child(info_layer, text_layer_get_layer(bluetooth_layer));
  
  battery_layer = text_layer_create((GRect) { .origin = { 100, 0 }, .size = { 44, 18 } });
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
  text_layer_set_background_color(battery_layer, GColorBlack);
  text_layer_set_text_color(battery_layer, GColorWhite);
  layer_add_child(info_layer, text_layer_get_layer(battery_layer));
  
  day_layer = text_layer_create((GRect) { .origin = { 0, 150 }, .size = { 44, 18 } });
  text_layer_set_font(day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(day_layer, GTextAlignmentCenter);
  text_layer_set_background_color(day_layer, GColorBlack);
  text_layer_set_text_color(day_layer, GColorWhite);
  layer_add_child(info_layer, text_layer_get_layer(day_layer)); 
  
  date_layer = text_layer_create((GRect) { .origin = { 100, 150 }, .size = { 44, 18 } });
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_background_color(date_layer, GColorBlack);
  text_layer_set_text_color(date_layer, GColorWhite);
  layer_add_child(info_layer, text_layer_get_layer(date_layer));  
  
  if (info == info_on)
    enable_info();
  else
    disable_info();
  
  // Init digital time
  digital_container = layer_create(window_bounds);  
  layer_add_child(window_layer, digital_container);
  
  digital_layer = text_layer_create((GRect) { .origin = { 48, 0 }, .size = { 48, 18 } });
  text_layer_set_font(digital_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(digital_layer, GTextAlignmentCenter);
  text_layer_set_background_color(digital_layer, GColorBlack);
  text_layer_set_text_color(digital_layer, GColorWhite);
  layer_add_child(digital_container, text_layer_get_layer(digital_layer));
  
  if (digital == digital_off)
    disable_digital();
  else
    enable_digital();
  
  // Init hour hand
  white_hour_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HOUR_HAND_WHITE);
  white_hour_layer = rot_bitmap_layer_create(white_hour_bitmap);
  rot_bitmap_set_compositing_mode(white_hour_layer, GCompOpOr);
  rot_bitmap_set_src_ic(white_hour_layer, GPoint(33, 40));
  layer_add_child(window_layer, bitmap_layer_get_layer((BitmapLayer*) white_hour_layer));
    
  black_hour_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HOUR_HAND_BLACK);
  black_hour_layer = rot_bitmap_layer_create(black_hour_bitmap);
  rot_bitmap_set_compositing_mode(black_hour_layer, GCompOpClear);
  rot_bitmap_set_src_ic(black_hour_layer, GPoint(33, 40));
  layer_add_child(window_layer, bitmap_layer_get_layer((BitmapLayer*) black_hour_layer));
  
  // Init minute hand
  white_minute_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MINUTE_HAND_WHITE);
  white_minute_layer = rot_bitmap_layer_create(white_minute_bitmap);
  rot_bitmap_set_compositing_mode(white_minute_layer, GCompOpOr);
  rot_bitmap_set_src_ic(white_minute_layer, GPoint(16, 60));
  layer_add_child(window_layer, bitmap_layer_get_layer((BitmapLayer*) white_minute_layer));
    
  black_minute_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MINUTE_HAND_BLACK);
  black_minute_layer = rot_bitmap_layer_create(black_minute_bitmap);
  rot_bitmap_set_compositing_mode(black_minute_layer, GCompOpClear);
  rot_bitmap_set_src_ic(black_minute_layer, GPoint(16, 60));
  layer_add_child(window_layer, bitmap_layer_get_layer((BitmapLayer*) black_minute_layer));
  
  // Init second hand
  white_second_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SECOND_HAND_WHITE);
  white_second_layer = rot_bitmap_layer_create(white_second_bitmap);
  rot_bitmap_set_compositing_mode(white_second_layer, GCompOpOr);
  rot_bitmap_set_src_ic(white_second_layer, GPoint(7, 44));
    
  black_second_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SECOND_HAND_BLACK);
  black_second_layer = rot_bitmap_layer_create(black_second_bitmap);
  rot_bitmap_set_compositing_mode(black_second_layer, GCompOpClear);
  rot_bitmap_set_src_ic(black_second_layer, GPoint(7, 44));
  
  // Start the proper timer
  if (seconds == seconds_on)
    enable_seconds();
  else
    disable_seconds();
}

void deinit(void) {
  app_sync_deinit(&app);
  
  tick_timer_service_unsubscribe();
  
  gbitmap_destroy(background_bitmap);
  
  gbitmap_destroy(black_hour_bitmap);
  gbitmap_destroy(white_hour_bitmap);
  gbitmap_destroy(black_minute_bitmap);
  gbitmap_destroy(white_minute_bitmap);
  gbitmap_destroy(black_second_bitmap);
  gbitmap_destroy(white_second_bitmap);
  
  bitmap_layer_destroy(background_layer); 
  
  rot_bitmap_layer_destroy(black_hour_layer);
  rot_bitmap_layer_destroy(white_hour_layer);
  rot_bitmap_layer_destroy(black_minute_layer);
  rot_bitmap_layer_destroy(white_minute_layer);
  rot_bitmap_layer_destroy(black_second_layer);
  rot_bitmap_layer_destroy(white_second_layer);
  
  layer_destroy(info_container);
  layer_destroy(info_layer);   
  layer_destroy(digital_container);
  
  text_layer_destroy(bluetooth_layer);
  text_layer_destroy(battery_layer);
  text_layer_destroy(day_layer);
  text_layer_destroy(date_layer);
  
  text_layer_destroy(digital_layer);
                   
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}