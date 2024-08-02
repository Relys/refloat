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

#include "haptic_buzz.h"

void haptic_buzz_configure(HapticBuzz *haptic_buzz, RefloatConfig *float_conf) {
    haptic_buzz->haptic_buzz_intensity = float_conf->haptic_buzz_intensity;
    haptic_buzz->haptic_buzz_min = float_conf->haptic_buzz_min;
    haptic_buzz->haptic_buzz_duty = float_conf->haptic_buzz_duty;
    haptic_buzz->haptic_buzz_hv = float_conf->haptic_buzz_hv;
    haptic_buzz->haptic_buzz_lv = float_conf->haptic_buzz_lv;
    haptic_buzz->haptic_buzz_temp = float_conf->haptic_buzz_temp;
}

void haptic_buzz_reset(HapticBuzz *haptic_buzz, float current_time) {
    haptic_buzz->haptic_tone_in_progress = false;
    haptic_buzz->haptic_timer = current_time;
    haptic_buzz->applied_haptic_current = 0;
}

float haptic_buzz_update(
    HapticBuzz *haptic_buzz,
    State *state,
    float current_time,
    float abs_erpm,
    float startup_click_current,
    float note_period,
    bool brake
) {
    if (state->mode == MODE_FLYWHEEL) {
        return 0;
    }
    if (((state->sat > SAT_NONE) && (state->state == STATE_RUNNING))) {

        if (state->sat == SAT_PB_DUTY) {
            haptic_buzz->haptic_type = haptic_buzz->haptic_buzz_duty;
        } else if (state->sat == SAT_PB_HIGH_VOLTAGE) {
            haptic_buzz->haptic_type = haptic_buzz->haptic_buzz_hv;
        } else if (state->sat == SAT_PB_LOW_VOLTAGE) {
            haptic_buzz->haptic_type = haptic_buzz->haptic_buzz_lv;
        } else if (state->sat == SAT_PB_TEMPERATURE) {
            haptic_buzz->haptic_type = haptic_buzz->haptic_buzz_temp;
        } else {
            haptic_buzz->haptic_type = HAPTIC_BUZZ_NONE;
        }

        // This kicks it off till at least one ~300ms tone is completed
        if (haptic_buzz->haptic_type != HAPTIC_BUZZ_NONE) {
            haptic_buzz->haptic_tone_in_progress = true;
        }
    }

    if (haptic_buzz->haptic_tone_in_progress || brake) {
        haptic_buzz->haptic_counter += 1;

        float buzz_current = fminf(20, haptic_buzz->haptic_buzz_intensity);
        // small periods (1,2) produce audible tone, higher periods produce vibration
        int buzz_period = haptic_buzz->haptic_type;
        if (haptic_buzz->haptic_type == HAPTIC_BUZZ_ALTERNATING) {
            buzz_period = 1;
        }

        // alternate frequencies, depending on "mode"
        buzz_period += haptic_buzz->haptic_mode;

        if (brake) {
            // This is to emulate the equivalent of "stop click"
            buzz_current = fmaxf(3, startup_click_current * 0.8);
            buzz_current = fminf(10, buzz_current);
            buzz_period = 0;
        } else if ((abs_erpm < 10000) && (buzz_current > 5)) {
            // scale high currents down to as low as 5A for lower erpms
            buzz_current = fmaxf(haptic_buzz->haptic_buzz_min, abs_erpm / 10000 * buzz_current);
        }

        if (haptic_buzz->haptic_counter > buzz_period) {
            haptic_buzz->haptic_counter = 0;
        }

        if (haptic_buzz->haptic_counter == 0) {
            if (haptic_buzz->applied_haptic_current > 0) {
                haptic_buzz->applied_haptic_current = -buzz_current;
            } else {
                haptic_buzz->applied_haptic_current = buzz_current;
            }

            if (fabsf(haptic_buzz->haptic_timer - current_time) > note_period) {
                haptic_buzz->haptic_tone_in_progress = false;
                if (brake) {
                    haptic_buzz->haptic_mode += 1;
                } else {
                    if (haptic_buzz->haptic_type == HAPTIC_BUZZ_ALTERNATING) {
                        haptic_buzz->haptic_mode = 5 - haptic_buzz->haptic_mode;
                    } else {
                        haptic_buzz->haptic_mode = 1 - haptic_buzz->haptic_mode;
                    }
                }

                haptic_buzz->haptic_timer = current_time;
            }
        }
    } else {
        haptic_buzz->haptic_mode = 0;
        haptic_buzz->haptic_counter = 0;
        haptic_buzz->haptic_timer = current_time;
        haptic_buzz->applied_haptic_current = 0;
    }
    return haptic_buzz->applied_haptic_current;
}
