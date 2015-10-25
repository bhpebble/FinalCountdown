#include <pebble.h>

Window *main_window;
TextLayer *bg_layer;
TextLayer *time_layer;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create a black background
  bg_layer = text_layer_create(
    GRect(0, 0, bounds.size.w, bounds.size.h));
  text_layer_set_background_color(bg_layer, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(bg_layer));
  
  // Create the TextLayer with specific bounds
  time_layer = text_layer_create(
      GRect(0, 63, bounds.size.w, 36));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(time_layer, GColorDarkGray);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_text(time_layer, "00:00");
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(time_layer);
}

void handle_init(void) {
  main_window = window_create();

  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  window_stack_push(main_window, true);
  update_time();
}

void handle_deinit(void) {
  text_layer_destroy(time_layer);
  window_destroy(main_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
