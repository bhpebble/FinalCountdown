#include <pebble.h>
#include "arc.c"

static const GPoint center = { 72, 84 };

Window *main_window;
TextLayer *bg_layer;
TextLayer *time_layer;
static Layer *draw_layer;

int radius = 60;
int thickness = 1;
int start_angle = 0;
int end_angle = 0;

static void updateScreen(Layer *layer, GContext *ctx) {
	custom_draw_arc(ctx, center, radius, thickness, start_angle, end_angle, GColorDarkGray);
	custom_draw_arc(ctx, center, radius+1, thickness*2+1, 0, TRIG_MAX_ANGLE/2, GColorPurple);
  graphics_context_set_antialiased(ctx, true);
}

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
  
  if (units_changed & DAY_UNIT) {
    layer_mark_dirty(draw_layer);
	}
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // set up a black background
  bg_layer = text_layer_create(
    GRect(0, 0, bounds.size.w, bounds.size.h));
  text_layer_set_background_color(bg_layer, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(bg_layer));
  
  // set up time display layer
  time_layer = text_layer_create(
      GRect(0, 63, bounds.size.w, 36));
  text_layer_set_background_color(time_layer, GColorDarkGray);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_text(time_layer, "00:00");
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  
  // set up layer and draw initial ring
  draw_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(draw_layer, updateScreen);
	layer_add_child(window_layer, draw_layer);
  layer_mark_dirty(draw_layer);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(time_layer);
  text_layer_destroy(bg_layer);
  layer_destroy(draw_layer);
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
