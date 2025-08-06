#include "pointing_device.h"
#include "quantum.h"
#include <math.h>

#define TB_LEFT  PAL_LINE(GPIOC, 11U)
#define TB_RIGHT PAL_LINE(GPIOC, 9U)
#define TB_UP    PAL_LINE(GPIOC, 8U)
#define TB_DOWN  PAL_LINE(GPIOC, 10U)

enum { AXIS_X = 0, AXIS_Y, AXIS_NUM, AXIS_MASK = 0x03 };

enum { TB_INCR = 0x10, TB_DECR = 0x20, TB_MASK = 0x30 };

enum { MODE_WHEEL, MODE_MOUSE };

static bool select_button_pressed = false;

static int8_t distances[AXIS_NUM] = {0};

static void trackball_cb(void* arg) {
  int axis = (int)arg & AXIS_MASK;
  int delta = (((int)arg & TB_MASK) == TB_DECR) ? -1 : 1;
  chSysLockFromISR();
  distances[axis] += delta;
  chSysUnlockFromISR();
}

void pointing_device_driver_init(void) {
  palSetLineMode(TB_LEFT, PAL_MODE_INPUT_PULLUP);
  palSetLineMode(TB_RIGHT, PAL_MODE_INPUT_PULLUP);
  palSetLineMode(TB_UP, PAL_MODE_INPUT_PULLUP);
  palSetLineMode(TB_DOWN, PAL_MODE_INPUT_PULLUP);

  palEnableLineEvent(TB_LEFT, PAL_EVENT_MODE_BOTH_EDGES);
  palEnableLineEvent(TB_RIGHT, PAL_EVENT_MODE_BOTH_EDGES);
  palEnableLineEvent(TB_UP, PAL_EVENT_MODE_BOTH_EDGES);
  palEnableLineEvent(TB_DOWN, PAL_EVENT_MODE_BOTH_EDGES);

  palSetLineCallback(TB_LEFT, trackball_cb, (void*)(AXIS_X|TB_DECR));
  palSetLineCallback(TB_RIGHT, trackball_cb, (void*)(AXIS_X|TB_INCR));
  palSetLineCallback(TB_UP, trackball_cb, (void*)(AXIS_Y|TB_DECR));
  palSetLineCallback(TB_DOWN, trackball_cb, (void*)(AXIS_Y|TB_INCR));
}

#define TRACKBALL_SCALE 1

report_mouse_t pointing_device_driver_get_report(report_mouse_t mouse_report) {
  chSysLock();

  const uint8_t mode = select_button_pressed ? MODE_WHEEL : MODE_MOUSE;
  switch(mode){
    case MODE_MOUSE: {
      mouse_report.x += TRACKBALL_SCALE*distances[AXIS_X];
      mouse_report.y += TRACKBALL_SCALE*distances[AXIS_Y];
      break;
    }
    case MODE_WHEEL: {
      mouse_report.h += TRACKBALL_SCALE*distances[AXIS_X];
      mouse_report.v += TRACKBALL_SCALE*distances[AXIS_Y];
      break;
    }
  }

  distances[AXIS_X] = 0;
  distances[AXIS_Y] = 0;

  chSysUnlock();

  return mouse_report;
}

uint16_t pointing_device_driver_get_cpi(void) {
  // TODO?
  return 0;
}

void pointing_device_driver_set_cpi(uint16_t cpi) {
  // TODO?
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
  if (!process_record_user(keycode, record)) {
    return false;
  }

  if (keycode == JS_4) {
    select_button_pressed = record->event.pressed;
  }

  return true;
}
