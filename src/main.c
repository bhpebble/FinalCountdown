#include <pebble.h>
#include "arc.c"

static const GPoint center = { 72, 84 };

struct arc counter_config;
time_t start_time = 1444633200/60/60/24;
time_t end_time = 1449907200/60/60/24;

Window *main_window;
TextLayer *bg_layer;
TextLayer *time_layer;
static Layer *main_draw_layer;
static Layer *bl_draw_layer;
static Layer *bc_draw_layer;
static Layer *br_draw_layer;

static TextLayer *temp_draw_layer_red;
static TextLayer *temp_draw_layer_green;
static TextLayer *temp_draw_layer_blue;

int radius = 60;
int lower_thickness = 1;
int higher_thickness_multiple = 4;
int start_angle = 0;
int end_angle = 0;


static void update_main_screen(Layer *layer, GContext *ctx) {
	custom_draw_arc(ctx, center, radius, lower_thickness, start_angle, end_angle, GColorDarkGray);
	custom_draw_arc_from_config(ctx, center, radius+2, lower_thickness*4+1, counter_config, GColorPurple);
  graphics_context_set_antialiased(ctx, true);
}

static void update_bottom_left_screen(Layer *layer, GContext *ctx) {
  GPoint c = {24,20};
	custom_draw_arc(ctx, c, 20, 1, 0,0, GColorDarkGray);
}

static void update_bottom_center_screen(Layer *layer, GContext *ctx) {
  GPoint c = {24,20};
	custom_draw_arc(ctx, c, 20, 1, 0,0, GColorDarkGray);
}

static void update_bottom_right_screen(Layer *layer, GContext *ctx) {
  GPoint c = {24,20};
	custom_draw_arc(ctx, c, 20, 1, 0,0, GColorDarkGray);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the arc_end_angle hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(time_layer, s_buffer);
}

static void update_counter() {
  int current_time = time(NULL)/60/60/24;
  int current_step = (int)current_time - (int)start_time;
  set_arc_step(&counter_config, current_step);
  
  layer_mark_dirty(main_draw_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

  if (units_changed == DAY_UNIT)
    update_counter();
}

static void main_window_load(Window *window) {
  // get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // set up a black background
  bg_layer = text_layer_create(
    GRect(0, 0, bounds.size.w, bounds.size.h));
  text_layer_set_background_color(bg_layer, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(bg_layer));
  
  // set up time display layer
  time_layer = text_layer_create(
      GRect(0, 43, bounds.size.w, 36));
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_text(time_layer, "00:00");
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  
  // set up counter layer and draw initial ring
  main_draw_layer = layer_create(GRect(0, -20, 144, 168));
	layer_set_update_proc(main_draw_layer, update_main_screen);
	layer_add_child(window_layer, main_draw_layer);

  int arc_slices = (int)end_time - (int)start_time;
  counter_config = create_arc_config(arc_slices);
  set_arc_offset(&counter_config, -angle_90);
  update_counter();
  
  // set up bottom left data layer and draw initial ring
  bl_draw_layer = layer_create(GRect(0, 127, 48, 41));
	layer_set_update_proc(bl_draw_layer, update_bottom_left_screen);
	layer_add_child(window_layer, bl_draw_layer);
  layer_mark_dirty(bl_draw_layer);
  
  // set up bottom center data layer and draw initial ring
  bc_draw_layer = layer_create(GRect(48, 127, 48, 41));
	layer_set_update_proc(bc_draw_layer, update_bottom_center_screen);
	layer_add_child(window_layer, bc_draw_layer);
  layer_mark_dirty(bc_draw_layer);
  
  // set up bottom center data layer and draw initial ring
  br_draw_layer = layer_create(GRect(96, 127, 48, 41));
	layer_set_update_proc(br_draw_layer, update_bottom_right_screen);
	layer_add_child(window_layer, br_draw_layer);
  layer_mark_dirty(br_draw_layer); 
}

static void main_window_unload(Window *window) {
  text_layer_destroy(time_layer);
  text_layer_destroy(bg_layer);
  layer_destroy(main_draw_layer);
}

void handle_init(void) {
  main_window = window_create();

  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  //tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  window_stack_push(main_window, true);
  update_time();
}

void handle_deinit(void) {
  window_destroy(main_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
