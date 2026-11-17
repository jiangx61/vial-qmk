#include "quantum.h"
#include "os_detection.h"

// Track physical keys, including multiple GUI Mod-Taps on the same side.
// Dedicated Win keys do not activate this guard.
static matrix_row_t gui_mod_tap_keys[MATRIX_ROWS];

static bool gui_mod_tap_active(void) {
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        if (gui_mod_tap_keys[row]) return true;
    }
    return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Both left and right GUI use MOD_LGUI in QMK's five-bit modifier field;
    // the high bit selects the right side. Only hold events activate the guard.
    if (IS_QK_MOD_TAP(keycode) && (QK_MOD_TAP_GET_MODS(keycode) & MOD_LGUI) && record->tap.count == 0 &&
        record->event.key.row < MATRIX_ROWS && record->event.key.col < MATRIX_COLS) {
        const matrix_row_t bit = (matrix_row_t)1 << record->event.key.col;
        if (record->event.pressed) {
            // Also cover L/U pressed before the GUI hold: remove them from
            // the normal/NKRO report before QMK registers the modifier.
            if (detected_host_os() == OS_WINDOWS) {
                unregister_code(KC_L);
                unregister_code(KC_U);
            }
            gui_mod_tap_keys[record->event.key.row] |= bit;
        } else {
            gui_mod_tap_keys[record->event.key.row] &= ~bit;
        }
        return true;
    }

    uint16_t basic = keycode;
    if (IS_QK_MOD_TAP(keycode)) {
        if (!record->tap.count) return true;
        basic = QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
    } else if (IS_QK_LAYER_TAP(keycode)) {
        if (!record->tap.count) return true;
        basic = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
    } else if (IS_QK_MODS(keycode)) {
        basic = QK_MODS_GET_BASIC_KEYCODE(keycode);
    }

    if (record->event.pressed && (basic == KC_L || basic == KC_U) && detected_host_os() == OS_WINDOWS &&
        gui_mod_tap_active() && ((get_mods() | get_weak_mods() | get_oneshot_mods()) & MOD_MASK_GUI)) {
        return false;
    }

    // Always process releases, even if GUI was released before L/U.
    return true;
}
