/*
 *
 * Copyright (c) 2023 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/random/random.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/display.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/split/bluetooth/peripheral.h>
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>

#include "peripheral_status.h"

LV_IMG_DECLARE(reaction_diffusion_3);

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct peripheral_status_state {
    bool connected;
};

static void draw_top(lv_obj_t *widget, lv_color_t cbuf[], const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 0);

    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_RIGHT);
    lv_draw_rect_dsc_t rect_black_dsc;
    init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);

    // Fill background
    lv_canvas_draw_rect(canvas, 0, 0, CANVAS_SIZE, CANVAS_SIZE, &rect_black_dsc);

    // Draw battery
    // Need to fix this, only works with 12 (or maybe below)
    // Larger fonts overflow. 14 is close, probably works for non-100%. Could just keep it at 2 digits??
    draw_battery_sized(canvas, state, &lv_font_montserrat_14);

    // Draw output status
    lv_canvas_draw_text(canvas, 0, 0, CANVAS_SIZE, &label_dsc,
                        state->connected ? LV_SYMBOL_BLUETOOTH : LV_SYMBOL_CLOSE);
    
    lv_draw_label_dsc_t label_dsc_center;
    init_label_dsc(&label_dsc_center, LVGL_FOREGROUND, &lv_font_montserrat_28,
                   LV_TEXT_ALIGN_CENTER);
    char name[] = "BEN";
    lv_canvas_draw_text(canvas, 0, 28, CANVAS_SIZE, &label_dsc_center, name);

    // Rotate canvas
    rotate_canvas(canvas, cbuf);
}

static void set_battery_status(struct zmk_widget_status *widget,
                               struct battery_status_state state) {
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    widget->state.charging = state.usb_present;
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

    widget->state.battery = state.level;

    draw_top(widget->obj, widget->cbuf, &widget->state);
}

static void battery_status_update_cb(struct battery_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_status(widget, state); }
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    return (struct battery_status_state){
        .level = zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state)

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

static struct peripheral_status_state get_state(const zmk_event_t *_eh) {
    return (struct peripheral_status_state){.connected = zmk_split_bt_peripheral_is_connected()};
}

static void set_connection_status(struct zmk_widget_status *widget,
                                  struct peripheral_status_state state) {
    widget->state.connected = state.connected;

    draw_top(widget->obj, widget->cbuf, &widget->state);
}

static void output_status_update_cb(struct peripheral_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_connection_status(widget, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_peripheral_status, struct peripheral_status_state,
                            output_status_update_cb, get_state)
ZMK_SUBSCRIPTION(widget_peripheral_status, zmk_split_peripheral_status_changed);

int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 160, 68);
    lv_obj_t *top = lv_canvas_create(widget->obj);
    lv_obj_align(top, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_canvas_set_buffer(top, widget->cbuf, CANVAS_SIZE, CANVAS_SIZE, LV_IMG_CF_TRUE_COLOR);

    lv_obj_t *art = lv_img_create(widget->obj);
    lv_img_set_src(art, &reaction_diffusion_3);
    lv_obj_align(art, LV_ALIGN_TOP_LEFT, 0, 0);

    sys_slist_append(&widgets, &widget->node);
    widget_battery_status_init();
    widget_peripheral_status_init();

    return 0;
}

lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget) { return widget->obj; }

#ifndef LV_ATTRIBUTE_IMG_REACTION_DIFFUSION_3
#define LV_ATTRIBUTE_IMG_REACTION_DIFFUSION_3
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_REACTION_DIFFUSION_3 uint8_t
    reaction_diffusion_3_map[] = {
#if CONFIG_NICE_VIEW_WIDGET_INVERTED
        0xff, 0xff, 0xff, 0xff, /*Color of index 0*/
        0x00, 0x00, 0x00, 0xff, /*Color of index 1*/
#else
        0x00, 0x00, 0x00, 0xff, /*Color of index 0*/
        0xff, 0xff, 0xff, 0xff, /*Color of index 1*/
#endif

        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0x9f, 0x9f,
        0xff, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0x1f, 0x8f, 0xff, 0xe1, 0xf9, 0xff,
        0xff, 0xc7, 0xfc, 0xff, 0xff, 0x0f, 0x0f, 0x1f, 0xc1, 0xf0, 0xff, 0x8f, 0x83, 0xfc, 0xdf,
        0xff, 0x0f, 0x0f, 0x0f, 0x83, 0xf0, 0xff, 0x07, 0x87, 0xfc, 0x8f, 0xff, 0x0e, 0x0e, 0x0f,
        0x83, 0xf0, 0xff, 0x07, 0x87, 0xfc, 0x87, 0xff, 0x00, 0x1e, 0x1f, 0x07, 0xe0, 0x7f, 0x87,
        0x07, 0xfc, 0x83, 0xff, 0x00, 0x1e, 0x1f, 0x07, 0x80, 0x3f, 0x87, 0x07, 0xdc, 0xc0, 0xfe,
        0x00, 0x3c, 0x1e, 0x0f, 0x00, 0x0f, 0x07, 0x0f, 0x8c, 0xc0, 0x00, 0x00, 0x7c, 0x1c, 0x1f,
        0x00, 0x00, 0x07, 0x0f, 0x84, 0xe0, 0x00, 0x00, 0xf0, 0x00, 0x1e, 0x0e, 0x00, 0x0f, 0x0f,
        0x04, 0xf0, 0x00, 0x3f, 0xe0, 0x00, 0x3e, 0x1f, 0x00, 0x0f, 0x07, 0x04, 0xfc, 0x00, 0xff,
        0xe0, 0x00, 0x3e, 0x1f, 0x00, 0x0f, 0x00, 0x0c, 0xff, 0xc1, 0xff, 0xe0, 0xe0, 0x3c, 0x1e,
        0x0f, 0x3f, 0x00, 0x0c, 0xff, 0xff, 0xf0, 0xf3, 0xf8, 0x3c, 0x3e, 0x1f, 0xfe, 0x00, 0x1c,
        0xff, 0xff, 0xc0, 0x3f, 0xfc, 0x08, 0x3c, 0x1f, 0xfc, 0x00, 0x3c, 0xf0, 0x3f, 0x80, 0x1f,
        0xbe, 0x00, 0x3c, 0x1f, 0xf8, 0x0e, 0xfc, 0xe0, 0x0f, 0x00, 0x0e, 0x1e, 0x00, 0x3c, 0x1f,
        0x80, 0x3f, 0xfc, 0xe0, 0x0f, 0x02, 0x0e, 0x0f, 0x00, 0x1e, 0x00, 0x00, 0x7f, 0xfc, 0xf0,
        0x07, 0x0f, 0x0e, 0x0f, 0xc0, 0x1f, 0x00, 0x00, 0xff, 0xfc, 0xff, 0x07, 0x0f, 0x07, 0x07,
        0xfe, 0x0f, 0x00, 0x03, 0xfc, 0x7c, 0xff, 0x87, 0x0f, 0x87, 0x07, 0xff, 0x07, 0x80, 0x0f,
        0xf0, 0x3c, 0xff, 0x87, 0x07, 0x87, 0x81, 0xff, 0x07, 0x83, 0xff, 0xc0, 0x1c, 0xff, 0x87,
        0x87, 0x87, 0xc0, 0x0f, 0x87, 0x83, 0xff, 0x00, 0x3c, 0xff, 0x07, 0x87, 0x87, 0xc0, 0x07,
        0x87, 0x83, 0xfe, 0x00, 0x7c, 0xf0, 0x07, 0x87, 0x87, 0xc0, 0x03, 0x83, 0x81, 0xf8, 0x03,
        0xfc, 0xe0, 0x0f, 0x87, 0x87, 0xc0, 0x03, 0xc3, 0xc0, 0xf0, 0x0f, 0xfc, 0xe0, 0x0f, 0x87,
        0x87, 0xc3, 0x83, 0xc3, 0xe0, 0x00, 0x3f, 0xfc, 0xf0, 0x0f, 0x87, 0x87, 0xc3, 0xc3, 0xc3,
        0xf0, 0x00, 0x7f, 0xcc, 0xff, 0x0f, 0x87, 0x87, 0x83, 0xc3, 0xc1, 0xf8, 0x01, 0xff, 0x04,
        0xff, 0x0f, 0x07, 0x83, 0x83, 0xc3, 0xc0, 0xfe, 0x03, 0xfc, 0x04, 0xff, 0x0f, 0x07, 0x80,
        0x07, 0xc3, 0xc0, 0x3f, 0x9f, 0xe0, 0x04, 0xff, 0x0f, 0x0f, 0x80, 0x07, 0x83, 0xc0, 0x0f,
        0xff, 0x80, 0x0c, 0xff, 0x0f, 0x0f, 0x00, 0x07, 0x87, 0x80, 0x03, 0xfe, 0x00, 0x3c, 0xfe,
        0x0e, 0x0e, 0x04, 0x0f, 0x07, 0x82, 0x01, 0xfc, 0x00, 0xfc, 0xfe, 0x1e, 0x1e, 0x0f, 0xff,
        0x07, 0x07, 0x80, 0xf8, 0x07, 0xfc, 0xfc, 0x1e, 0x1e, 0x0f, 0xfe, 0x0f, 0x0f, 0xe0, 0x78,
        0x3f, 0xfc, 0xf8, 0x3e, 0x1e, 0x0f, 0xfc, 0x0f, 0x0f, 0xf0, 0x70, 0x7f, 0xfc, 0xc0, 0x3e,
        0x1e, 0x07, 0xf8, 0x1f, 0x07, 0xf8, 0x70, 0x7f, 0xfc, 0x80, 0x3e, 0x1f, 0x00, 0x60, 0x3f,
        0x00, 0x38, 0x38, 0x7c, 0xfc, 0x80, 0x1e, 0x0f, 0x80, 0x00, 0x7e, 0x00, 0x3c, 0x38, 0x00,
        0x7c, 0x80, 0x1e, 0x07, 0xc0, 0x00, 0xfc, 0x00, 0x1c, 0x38, 0x00, 0x7c, 0xde, 0x0f, 0x07,
        0xf0, 0x01, 0xfc, 0x00, 0x3c, 0x3c, 0x00, 0x7c, 0xff, 0x0f, 0x83, 0xfe, 0x01, 0xf8, 0x3e,
        0x78, 0x3e, 0x00, 0xfc, 0xff, 0x0f, 0xc1, 0xef, 0xe0, 0x00, 0x7f, 0xf8, 0x7f, 0x87, 0xfc,
        0xff, 0x07, 0xe0, 0x07, 0xf0, 0x00, 0x7f, 0xf0, 0x7f, 0xff, 0xfc, 0xfe, 0x03, 0xe0, 0x07,
        0xf0, 0x00, 0x7c, 0x60, 0xff, 0xff, 0xfc, 0xe0, 0x01, 0xe0, 0x07, 0xf0, 0x00, 0x78, 0x01,
        0xe1, 0xff, 0xfc, 0xc0, 0x00, 0xc0, 0x07, 0xe0, 0x70, 0x38, 0x03, 0xc0, 0x00, 0x7c, 0x80,
        0x20, 0x01, 0xff, 0x81, 0xf8, 0x38, 0x03, 0xc0, 0x00, 0x7c, 0x80, 0x78, 0x03, 0xf8, 0x01,
        0xfc, 0x1c, 0x03, 0xc0, 0x00, 0x3c, 0xc0, 0x7c, 0x07, 0xf0, 0x00, 0xfe, 0x0f, 0x83, 0xc0,
        0x0e, 0x3c, 0xf8, 0x7e, 0x0f, 0xc0, 0x00, 0x7e, 0x0f, 0xc1, 0xe1, 0xff, 0x7c, 0xf8, 0x3e,
        0x0f, 0x80, 0x00, 0x1f, 0x07, 0xc1, 0xe1, 0xff, 0xfc, 0xfc, 0x3e, 0x0f, 0x01, 0xf8, 0x0f,
        0x87, 0xe0, 0xe0, 0xff, 0xfc, 0xfc, 0x1e, 0x0e, 0x07, 0xfc, 0x07, 0x83, 0xe0, 0xe0, 0x7f,
        0xfc, 0xfc, 0x1e, 0x0e, 0x0f, 0xfe, 0x07, 0x83, 0xe0, 0xf0, 0x06, 0x0c, 0xfe, 0x0e, 0x0e,
        0x0f, 0xff, 0x83, 0xc3, 0xe0, 0xf0, 0x00, 0x0c, 0xfe, 0x0f, 0x0f, 0x0e, 0x0f, 0x83, 0xc1,
        0xc0, 0xf8, 0x00, 0x04, 0xff, 0x0f, 0x0f, 0x00, 0x07, 0xc1, 0xc0, 0x81, 0xff, 0x00, 0x0c,
        0xff, 0x0f, 0x07, 0x80, 0x03, 0xc1, 0xe0, 0x03, 0xff, 0xe0, 0x3c, 0xff, 0x07, 0x07, 0x80,
        0x03, 0xe1, 0xe0, 0x07, 0xff, 0xff, 0xfc, 0xff, 0x07, 0x87, 0x81, 0x83, 0xe1, 0xf0, 0x0f,
        0xff, 0xff, 0xfc, 0xff, 0x87, 0x87, 0xc3, 0xc7, 0xe0, 0xf8, 0x3f, 0xff, 0xff, 0xfc, 0xff,
        0x87, 0xcf, 0xe7, 0xcf, 0xe1, 0xf8, 0x3f, 0xff, 0xff, 0xfc, 0xff, 0xcf, 0xff, 0xff, 0xff,
        0xf3, 0xfc, 0x7f, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,
};

const lv_img_dsc_t reaction_diffusion_3 = {
    .header.cf = LV_IMG_CF_INDEXED_1BIT,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 86,
    .header.h = 68,
    .data_size = 756,
    .data = reaction_diffusion_3_map,
};
