# Copyright (c) 2024 The ZMK Contributors
# SPDX-License-Identifier: MIT

if(CONFIG_ZMK_BEHAVIOR_WHITELIST_LAYER)
zephyr_library_sources(src/behavior_whitelist_layer.c)
endif()
