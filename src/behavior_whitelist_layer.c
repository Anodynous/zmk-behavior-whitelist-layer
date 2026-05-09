/*
 * Copyright (c) 2024 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_whitelist_layer

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/keymap.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <string.h>

static const uint16_t whitelist_keycodes[] = {
    0x002C, /* SPACE */
    0x002A, /* BACKSPACE */
    0x004C, /* DELETE */
    0x0050, /* LEFT_ARROW */
    0x004F, /* RIGHT_ARROW */
    0x0052, /* UP_ARROW */
    0x0051, /* DOWN_ARROW */
    0x004B, /* PAGE_UP */
    0x004D, /* PAGE_DOWN */
    0x004A, /* HOME */
    0x004E, /* END */
};

static zmk_keymap_layer_id_t active_layer = 0;
static bool layer_active = false;
static uint32_t trigger_position = 0;

static int whitelist_layer_pressed(struct zmk_behavior_binding *binding,
                                   struct zmk_behavior_binding_event event) {
    active_layer = binding->param1;
    layer_active = true;
    trigger_position = event.position;
    return zmk_keymap_layer_activate(active_layer, false);
}

static int whitelist_layer_released(struct zmk_behavior_binding *binding,
                                    struct zmk_behavior_binding_event event) {
    if (!layer_active) {
        return ZMK_BEHAVIOR_OPAQUE;
    }
    int ret = zmk_keymap_layer_deactivate(active_layer, false);
    layer_active = false;
    active_layer = 0;
    trigger_position = 0;
    return ret;
}

static uint16_t get_keycode_at_pos(uint32_t position, zmk_keymap_layer_id_t layer) {
    const struct zmk_behavior_binding *binding;

    binding = zmk_keymap_get_behavior_binding(position, layer);
    if (binding == NULL) {
        return 0;
    }

    if (strcmp(binding->behavior_dev, "kp") == 0) {
        return binding->param1;
    }

    return 0;
}

static bool is_whitelisted(uint16_t keycode) {
    for (size_t i = 0; i < ARRAY_SIZE(whitelist_keycodes); i++) {
        if (whitelist_keycodes[i] == keycode) {
            return true;
        }
    }
    return false;
}

static int whitelist_layer_listener(const zmk_event_t *eh) {
    struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);

    if (ev == NULL || !layer_active || !ev->state) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (ev->position == trigger_position) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    uint16_t keycode = get_keycode_at_pos(ev->position, active_layer);

    if (keycode == 0 || !is_whitelisted(keycode)) {
        zmk_keymap_layer_deactivate(active_layer, false);
        layer_active = false;
        active_layer = 0;
        trigger_position = 0;
        ZMK_EVENT_RAISE(eh);
        return ZMK_EV_EVENT_CAPTURED;
    }

    return ZMK_EV_EVENT_BUBBLE;
}

static const struct behavior_driver_api whitelist_layer_api = {
    .binding_pressed = whitelist_layer_pressed,
    .binding_released = whitelist_layer_released,
};

ZMK_LISTENER(whitelist_layer, whitelist_layer_listener);
ZMK_SUBSCRIPTION(whitelist_layer, zmk_position_state_changed);

#define WHITELIST_LAYER_INST(n)                                                                    \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &whitelist_layer_api);

DT_INST_FOREACH_STATUS_OKAY(WHITELIST_LAYER_INST)
