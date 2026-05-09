# Copyright (c) 2024 The ZMK Contributors
# SPDX-License-Identifier: MIT

if ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_WHITELIST_LAYER app PRIVATE src/behavior_whitelist_layer.c)
endif()
