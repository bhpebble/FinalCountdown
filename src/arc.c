#include <pebble.h>
#include "arc.h"

static int angle_90 = TRIG_MAX_ANGLE / 4;
static int angle_180 = TRIG_MAX_ANGLE / 2;
static int angle_270 = 3 * TRIG_MAX_ANGLE / 4;

static struct arc create_arc_config(int num_slices) {
  
    struct arc config = {
    .num_slices = num_slices,
    .offset = 0,
    .start_angle = 0,
    .end_angle = 0,
    .current_step = 0,
    .step_angle = TRIG_MAX_ANGLE / num_slices
  };
  
  return config;
}

// Increments the step counter to the next step, then updates the end point of the arc
static void increment_arc_step(struct arc *config, int steps) {
  config->current_step += steps;
  update_arc_end_angle(config);
}

// Sets the current step to the provided step value, then updates the end point of the arc
static void set_arc_step(struct arc *config, int step) {
  config->current_step = step;
  update_arc_end_angle(config);
}

// Updates the end point of the arc based on number of slices and current step
static void update_arc_end_angle(struct arc *config) {
  config->end_angle = config->step_angle * config->current_step;
}

// Sets an offset (in number of slices) to apply to the arc
// Example: -TRIG_MAX_ANGLE / 4 will rotate the arc 90 degrees counter clockwise
static void set_arc_offset(struct arc *config, int offset) {
  config->offset= offset;
}

// Based on DrawArc function thanks to Cameron MacFarland (http://forums.getpebble.com/profile/12561/Cameron%20MacFarland)
// and Jnmattern (https://github.com/Jnmattern/Arc)
static void custom_draw_arc(GContext *ctx, GPoint center, int radius, int thickness, int start_angle, int end_angle, GColor c) {
	int32_t xmin = 65535000, xmax = -65535000, ymin = 65535000, ymax = -65535000;
	int32_t cosStart, sinStart, cosEnd, sinEnd;
	int32_t r, t;
	
	while (start_angle < 0) start_angle += TRIG_MAX_ANGLE;
	while (end_angle < 0) end_angle += TRIG_MAX_ANGLE;

	start_angle %= TRIG_MAX_ANGLE;
	end_angle %= TRIG_MAX_ANGLE;
	
	if (end_angle == 0) end_angle = TRIG_MAX_ANGLE;
	
	if (start_angle > end_angle) {
		custom_draw_arc(ctx, center, radius, thickness, start_angle, TRIG_MAX_ANGLE, c);
		custom_draw_arc(ctx, center, radius, thickness, 0, end_angle, c);
	} else {
		// Calculate bounding box for the arc to be drawn
		cosStart = cos_lookup(start_angle);
		sinStart = sin_lookup(start_angle);
		cosEnd = cos_lookup(end_angle);
		sinEnd = sin_lookup(end_angle);
		
		r = radius;
		// Point 1: radius & start_angle
		t = r * cosStart;
		if (t < xmin) xmin = t;
		if (t > xmax) xmax = t;
		t = r * sinStart;
		if (t < ymin) ymin = t;
		if (t > ymax) ymax = t;

		// Point 2: radius & end_angle
		t = r * cosEnd;
		if (t < xmin) xmin = t;
		if (t > xmax) xmax = t;
		t = r * sinEnd;
		if (t < ymin) ymin = t;
		if (t > ymax) ymax = t;
		
		r = radius - thickness;
		// Point 3: radius-thickness & start_angle
		t = r * cosStart;
		if (t < xmin) xmin = t;
		if (t > xmax) xmax = t;
		t = r * sinStart;
		if (t < ymin) ymin = t;
		if (t > ymax) ymax = t;

		// Point 4: radius-thickness & end_angle
		t = r * cosEnd;
		if (t < xmin) xmin = t;
		if (t > xmax) xmax = t;
		t = r * sinEnd;
		if (t < ymin) ymin = t;
		if (t > ymax) ymax = t;
		
		// Normalization
		xmin /= TRIG_MAX_RATIO;
		xmax /= TRIG_MAX_RATIO;
		ymin /= TRIG_MAX_RATIO;
		ymax /= TRIG_MAX_RATIO;
				
		// Corrections if arc crosses X or Y axis
		if ((start_angle < angle_90) && (end_angle > angle_90)) {
			ymax = radius;
		}
		
		if ((start_angle < angle_180) && (end_angle > angle_180)) {
			xmin = -radius;
		}
		
		if ((start_angle < angle_270) && (end_angle > angle_270)) {
			ymin = -radius;
		}
		
		// Slopes for the two sides of the arc
		float sslope = (float)cosStart/ (float)sinStart;
		float eslope = (float)cosEnd / (float)sinEnd;
	 
		if (end_angle == TRIG_MAX_ANGLE) eslope = -1000000;
	 
		int ir2 = (radius - thickness) * (radius - thickness);
		int or2 = radius * radius;
	 
		graphics_context_set_stroke_color(ctx, c);

		for (int x = xmin; x <= xmax; x++) {
			for (int y = ymin; y <= ymax; y++)
			{
				int x2 = x * x;
				int y2 = y * y;
	 
				if (
					(x2 + y2 < or2 && x2 + y2 >= ir2) && (
						(y > 0 && start_angle < angle_180 && x <= y * sslope) ||
						(y < 0 && start_angle > angle_180 && x >= y * sslope) ||
						(y < 0 && start_angle <= angle_180) ||
						(y == 0 && start_angle <= angle_180 && x < 0) ||
						(y == 0 && start_angle == 0 && x > 0)
					) && (
						(y > 0 && end_angle < angle_180 && x >= y * eslope) ||
						(y < 0 && end_angle > angle_180 && x <= y * eslope) ||
						(y > 0 && end_angle >= angle_180) ||
						(y == 0 && end_angle >= angle_180 && x < 0) ||
						(y == 0 && start_angle == 0 && x > 0)
					)
				)
				graphics_draw_pixel(ctx, GPoint(center.x+x, center.y+y));
			}
		}
	}
}

static void custom_draw_arc_from_config(GContext *ctx, GPoint center, int radius, int thickness, struct arc config, GColor c) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "CDAFC: %d, %d", config.start_angle + config.offset, config.end_angle + config.offset);
  custom_draw_arc(ctx, center, radius, thickness, config.start_angle + config.offset, config.end_angle + config.offset, c);
}
