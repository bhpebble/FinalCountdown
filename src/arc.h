#ifndef ARC_H_
#define ARC_H_

struct arc {
  int num_slices;
  int offset;
  int start_angle;
  int end_angle;
  int current_step;
  int step_angle;
};

static struct arc create_arc_config(int num_slices);
static void increment_arc_step(struct arc *config, int steps);
static void set_arc_step(struct arc *config, int step);
static void update_arc_end_angle(struct arc *config);

static void custom_draw_arc(GContext *ctx, GPoint center, int radius, int thickness, int start_angle, int end_angle, GColor c);
static void custom_draw_arc_from_config(GContext *ctx, GPoint center, int radius, int thickness, struct arc config, GColor c);
  
#endif