// Copyright 2024 Lukas Hrazky, Syler Clayton
//
// This file is part of the Refloat VESC package.
//
// Refloat VESC package is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// Refloat VESC package is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include "conf/datatypes.h"
#include "state.h"
#include <math.h>

typedef struct {
    float applied_haptic_current, haptic_timer;
    int haptic_counter, haptic_mode;
    HAPTIC_BUZZ_TYPE haptic_type;
    bool haptic_tone_in_progress;
    int haptic_buzz_intensity;
    int haptic_buzz_min;
    HAPTIC_BUZZ_TYPE haptic_buzz_duty;
    HAPTIC_BUZZ_TYPE haptic_buzz_hv;
    HAPTIC_BUZZ_TYPE haptic_buzz_lv;
    HAPTIC_BUZZ_TYPE haptic_buzz_temp;
} HapticBuzz;

void haptic_buzz_configure(HapticBuzz *haptic_buzz, RefloatConfig *float_conf);

void haptic_buzz_reset(HapticBuzz *haptic_buzz, float current_time);

float haptic_buzz_update(
    HapticBuzz *haptic_buzz,
    State *state,
    float current_time,
    float abs_erpm,
    float startup_click_current,
    float note_period,
    bool brake
);
