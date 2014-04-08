#include <pebble.h>

static Window *window;

static BitmapLayer *background_layer;
static RotBitmapLayer *black_hour_layer, *white_hour_layer, *black_minute_layer, *white_minute_layer, *black_second_layer, *white_second_layer;

static GBitmap *background_bitmap, *background_simple_bitmap;
static GBitmap *black_hour_bitmap, *white_hour_bitmap, *black_minute_bitmap, *white_minute_bitmap, *black_second_bitmap, *white_second_bitmap;

enum Settings { setting_seconds = 1, setting_background };
static enum SettingSeconds { seconds_on = 0, seconds_off } seconds;
static enum SettingBackground { background_rich = 0, background_simple } background;
static AppSync app;
static uint8_t buffer[256];

void update_time(struct tm* t) {
  // Update hour hand
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
  
  // Update minute hand
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
  
  // Update second hand
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

void handle_tick(struct tm *tick_time, TimeUnits units_changed) {  
  update_time(tick_time);
}

void enable_seconds() {  
  Layer *window_layer = window_get_root_layer(window);
  layer_add_child(window_layer, bitmap_layer_get_layer((BitmapLayer*) white_second_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer((BitmapLayer*) black_second_layer));
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);
  
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(SECOND_UNIT, &handle_tick);
}

void disable_seconds() {
  layer_remove_from_parent(bitmap_layer_get_layer((BitmapLayer*) white_second_layer));
  layer_remove_from_parent(bitmap_layer_get_layer((BitmapLayer*) black_second_layer));
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);
  
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_tick);
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
  
  // Set defaults
  seconds = persist_read_int(setting_seconds);
  background = persist_read_int(setting_background);
  
  // Load settings 
  Tuplet tuples[] = {
    TupletInteger(setting_seconds, seconds),
    TupletInteger(setting_background, background)
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
                   
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}