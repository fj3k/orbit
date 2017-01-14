#include <pebble.h>
#include "orbit.h"

static Window *s_window;
static Layer *s_simple_bg_layer, *s_hands_layer;

long a_rand(long seet) {
  static long seed = 100;
  if (seet > 0) seed = seet;
  seed = (((seed * 214013L + 2531011L) >> 16) & 32767);

  return (seed % 1000);
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  GRect bounds = layer_get_bounds(layer);
  GPoint centre = grect_center_point(&bounds);

  a_rand(256);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  for (int i = 0; i < 100; i++) {
    GPoint x = GPoint(
      a_rand(0) % bounds.size.w,
      a_rand(0) % bounds.size.h
    );
    graphics_draw_line(ctx, x, x);
  }

  graphics_context_set_antialiased(ctx, 1);
  graphics_context_set_fill_color(ctx, GColorYellow);
  graphics_fill_circle(ctx, centre, 9);
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint centre = grect_center_point(&bounds);

  graphics_context_set_antialiased(ctx, 1);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  graphics_context_set_fill_color(ctx, GColorYellow);
  graphics_fill_circle(ctx, centre, 9);

  int spacing = bounds.size.w / 7;
  int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  int32_t minute_angle = TRIG_MAX_ANGLE * (0. + t->tm_min) / 60 + second_angle / 60;
  int32_t hour_angle = TRIG_MAX_ANGLE * (t->tm_hour % 12) / 12 + minute_angle / 12;
  int32_t moon_angle = TRIG_MAX_ANGLE * ((t->tm_sec * 3) % 60) / 60;
  GPoint hourpos = GPoint(
    (int16_t)(sin_lookup(hour_angle) * (spacing * 3) / TRIG_MAX_RATIO) + centre.x,
    (int16_t)(-cos_lookup(hour_angle) * (spacing * 3) / TRIG_MAX_RATIO) + centre.y
  );
  GPoint minpos = GPoint(
    (int16_t)(sin_lookup(minute_angle) * (spacing * 2) / TRIG_MAX_RATIO) + centre.x,
    (int16_t)(-cos_lookup(minute_angle) * (spacing * 2) / TRIG_MAX_RATIO) + centre.y
  );
  GPoint secpos = GPoint(
    (int16_t)(sin_lookup(second_angle) * (spacing * 1) / TRIG_MAX_RATIO) + centre.x,
    (int16_t)(-cos_lookup(second_angle) * (spacing * 1) / TRIG_MAX_RATIO) + centre.y
  );
  GPoint moonpos = GPoint(
    (int16_t)(sin_lookup(moon_angle) * (spacing * 0.5) / TRIG_MAX_RATIO) + hourpos.x,
    (int16_t)(-cos_lookup(moon_angle) * (spacing * 0.5) / TRIG_MAX_RATIO) + hourpos.y
  );
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_circle(ctx, hourpos, 5);
  graphics_context_set_fill_color(ctx, GColorRajah);
  graphics_fill_circle(ctx, minpos, 4);
  graphics_context_set_fill_color(ctx, GColorChromeYellow);
  graphics_fill_circle(ctx, secpos, 2);
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_circle(ctx, moonpos, 1);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(s_window));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_simple_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
  layer_add_child(window_layer, s_simple_bg_layer);

  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_simple_bg_layer);
  layer_destroy(s_hands_layer);
}

static void init() {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  Layer *window_layer = window_get_root_layer(s_window);
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
