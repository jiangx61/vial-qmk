#include "test_common.hpp"

extern "C" {
#include "os_detection.h"
}

using testing::_;
using testing::InSequence;

static os_variant_t host_os = OS_WINDOWS;

extern "C" os_variant_t detected_host_os(void) {
    return host_os;
}

class WindowsGuiL : public TestFixture {
   protected:
    void check_gui_l(uint16_t gui_keycode, uint16_t l_keycode, os_variant_t os, bool gui_released_first, uint16_t basic = KC_L) {
        host_os = os;
        TestDriver driver;
        InSequence sequence;
        const uint16_t gui = (QK_MOD_TAP_GET_MODS(gui_keycode) & 0x10) ? KC_RGUI : KC_LGUI;
        auto mod = KeymapKey(0, 0, 0, gui_keycode);
        auto letter = KeymapKey(0, 1, 0, l_keycode);
        set_keymap({mod, letter});

        EXPECT_REPORT(driver, (gui));
        mod.press();
        idle_for(TAPPING_TERM + 1);

        if (os != OS_WINDOWS) {
            EXPECT_REPORT(driver, (gui, basic));
        }
        if (gui_released_first && l_keycode == basic) {
            letter.press();
            run_one_scan_loop();
            if (os != OS_WINDOWS) {
                EXPECT_REPORT(driver, (basic));
            } else {
                EXPECT_EMPTY_REPORT(driver);
            }
            mod.release();
            run_one_scan_loop();
            if (os != OS_WINDOWS) {
                EXPECT_EMPTY_REPORT(driver);
            }
            letter.release();
            run_one_scan_loop();
        } else {
            if (os != OS_WINDOWS) {
                EXPECT_REPORT(driver, (gui));
            }
            tap_key(letter);
            EXPECT_EMPTY_REPORT(driver);
            mod.release();
            run_one_scan_loop();
        }
        VERIFY_AND_CLEAR(driver);

        // Once the home-row hold is released, L must type again.
        EXPECT_REPORT(driver, (basic));
        EXPECT_EMPTY_REPORT(driver);
        tap_key(letter);
        VERIFY_AND_CLEAR(driver);
    }
};

TEST_F(WindowsGuiL, BothHomeRowModsBlockPlainAndModTapLOnWindows) {
    for (auto gui : {LGUI_T(KC_A), RGUI_T(KC_SCLN), LGUI_T(KC_Z), RGUI_T(KC_X), LGUI_T(KC_L)}) {
        for (uint16_t letter : std::initializer_list<uint16_t>{KC_L, RALT_T(KC_L), LT(1, KC_L)}) {
            check_gui_l(gui, letter, OS_WINDOWS, false);
        }
    }
}

TEST_F(WindowsGuiL, LinuxAndUnknownOsKeepSuperL) {
    for (auto os : {OS_LINUX, OS_UNSURE}) {
        for (auto gui : {LGUI_T(KC_A), RGUI_T(KC_SCLN), LGUI_T(KC_Z), RGUI_T(KC_X)}) {
            check_gui_l(gui, RALT_T(KC_L), os, false);
        }
    }
}

TEST_F(WindowsGuiL, ReleasingGuiBeforeLDoesNotLeaveLStuck) {
    check_gui_l(LGUI_T(KC_A), KC_L, OS_WINDOWS, true);
    check_gui_l(LGUI_T(KC_Z), KC_L, OS_WINDOWS, true);
    check_gui_l(RGUI_T(KC_SCLN), KC_L, OS_LINUX, true);
}

TEST_F(WindowsGuiL, LPressedBeforeHomeRowHoldIsReleasedBeforeGui) {
    host_os = OS_WINDOWS;
    TestDriver driver;
    InSequence sequence;
    auto letter = KeymapKey(0, 0, 0, KC_L);
    auto mod = KeymapKey(0, 1, 0, LGUI_T(KC_A));
    set_keymap({letter, mod});

    EXPECT_REPORT(driver, (KC_L));
    letter.press();
    run_one_scan_loop();
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_LGUI));
    mod.press();
    idle_for(TAPPING_TERM + 1);
    letter.release();
    run_one_scan_loop();
    EXPECT_EMPTY_REPORT(driver);
    mod.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

TEST_F(WindowsGuiL, HomeRowTapsOtherShortcutsAndLHoldStillWork) {
    host_os = OS_WINDOWS;
    TestDriver driver;
    InSequence sequence;
    auto left = KeymapKey(0, 0, 0, LGUI_T(KC_A));
    auto right = KeymapKey(0, 1, 0, RGUI_T(KC_SCLN));
    auto letter = KeymapKey(0, 2, 0, RALT_T(KC_L));
    auto other = KeymapKey(0, 3, 0, KC_E);
    set_keymap({left, right, letter, other});

    EXPECT_REPORT(driver, (KC_A));
    EXPECT_EMPTY_REPORT(driver);
    tap_key(left);
    EXPECT_REPORT(driver, (KC_SCLN));
    EXPECT_EMPTY_REPORT(driver);
    tap_key(right);
    idle_for(TAPPING_TERM + 1);

    EXPECT_REPORT(driver, (KC_LGUI));
    left.press();
    idle_for(TAPPING_TERM + 1);
    EXPECT_REPORT(driver, (KC_LGUI, KC_E));
    EXPECT_REPORT(driver, (KC_LGUI));
    tap_key(other);
    EXPECT_REPORT(driver, (KC_LGUI, KC_RALT));
    letter.press();
    idle_for(TAPPING_TERM + 1);
    EXPECT_REPORT(driver, (KC_LGUI));
    letter.release();
    run_one_scan_loop();
    EXPECT_EMPTY_REPORT(driver);
    left.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

TEST_F(WindowsGuiL, DedicatedWinKeyKeepsWinL) {
    host_os = OS_WINDOWS;
    TestDriver driver;
    InSequence sequence;
    auto win = KeymapKey(0, 0, 0, KC_LGUI);
    auto letter = KeymapKey(0, 1, 0, KC_L);
    set_keymap({win, letter});
    EXPECT_REPORT(driver, (KC_LGUI));
    win.press();
    run_one_scan_loop();
    EXPECT_REPORT(driver, (KC_LGUI, KC_L));
    EXPECT_REPORT(driver, (KC_LGUI));
    tap_key(letter);
    EXPECT_EMPTY_REPORT(driver);
    win.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

TEST_F(WindowsGuiL, ReleasingOneGuiModTapDoesNotForgetAnotherOnTheSameSide) {
    host_os = OS_WINDOWS;
    TestDriver driver;
    InSequence sequence;
    auto first = KeymapKey(0, 0, 0, LGUI_T(KC_A));
    auto second = KeymapKey(0, 1, 0, LGUI_T(KC_Z));
    auto win = KeymapKey(0, 2, 0, KC_LGUI);
    auto letter = KeymapKey(0, 3, 0, KC_L);
    set_keymap({first, second, win, letter});

    EXPECT_REPORT(driver, (KC_LGUI));
    first.press();
    idle_for(TAPPING_TERM + 1);
    second.press();
    idle_for(TAPPING_TERM + 1);
    // QMK clears the shared modifier when either hold is released.
    EXPECT_EMPTY_REPORT(driver);
    first.release();
    run_one_scan_loop();
    EXPECT_REPORT(driver, (KC_LGUI));
    win.press();
    run_one_scan_loop();
    // The second Mod-Tap is still held, so L must remain blocked.
    tap_key(letter);
    EXPECT_EMPTY_REPORT(driver);
    second.release();
    run_one_scan_loop();
    win.release();
    run_one_scan_loop();
    // Once all Mod-Taps are released, a dedicated Win key works normally.
    EXPECT_REPORT(driver, (KC_LGUI));
    win.press();
    run_one_scan_loop();
    EXPECT_REPORT(driver, (KC_LGUI, KC_L));
    EXPECT_REPORT(driver, (KC_LGUI));
    tap_key(letter);
    EXPECT_EMPTY_REPORT(driver);
    win.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

TEST_F(WindowsGuiL, WinUIsBlockedOnlyOnWindowsForAnyGuiModTap) {
    for (auto os : {OS_WINDOWS, OS_LINUX, OS_UNSURE}) {
        for (auto gui : {LGUI_T(KC_A), RGUI_T(KC_SCLN), LGUI_T(KC_Z), RGUI_T(KC_U)}) {
            for (uint16_t letter : std::initializer_list<uint16_t>{KC_U, RALT_T(KC_U), LT(1, KC_U)}) {
                check_gui_l(gui, letter, os, false, KC_U);
            }
        }
    }
    check_gui_l(LGUI_T(KC_Z), KC_U, OS_WINDOWS, true, KC_U);
}

TEST_F(WindowsGuiL, UPressedBeforeHomeRowHoldIsReleasedBeforeGui) {
    host_os = OS_WINDOWS;
    TestDriver driver;
    InSequence sequence;
    auto letter = KeymapKey(0, 0, 0, KC_U);
    auto mod = KeymapKey(0, 1, 0, LGUI_T(KC_A));
    set_keymap({letter, mod});

    EXPECT_REPORT(driver, (KC_U));
    letter.press();
    run_one_scan_loop();
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_LGUI));
    mod.press();
    idle_for(TAPPING_TERM + 1);
    letter.release();
    run_one_scan_loop();
    EXPECT_EMPTY_REPORT(driver);
    mod.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}

TEST_F(WindowsGuiL, DedicatedRightWinKeyKeepsWinU) {
    host_os = OS_WINDOWS;
    TestDriver driver;
    InSequence sequence;
    auto win = KeymapKey(0, 0, 0, KC_RGUI);
    auto letter = KeymapKey(0, 1, 0, KC_U);
    set_keymap({win, letter});
    EXPECT_REPORT(driver, (KC_RGUI));
    win.press();
    run_one_scan_loop();
    EXPECT_REPORT(driver, (KC_RGUI, KC_U));
    EXPECT_REPORT(driver, (KC_RGUI));
    tap_key(letter);
    EXPECT_EMPTY_REPORT(driver);
    win.release();
    run_one_scan_loop();
    VERIFY_AND_CLEAR(driver);
}
