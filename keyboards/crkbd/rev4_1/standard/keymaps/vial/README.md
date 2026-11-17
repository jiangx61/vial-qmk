# Corne v4.1 standard custom Vial firmware

This keymap is self-contained in this fork and builds as
`crkbd/rev4_1/standard:vial`. It does not require the external `kbd_firmware`
repository or a generated `keyboards/tmp` directory. Other Corne revisions
retain their upstream keymaps. Eight dynamic layers are enabled.

## Fixed Windows Mod-Tap shortcut guard

The firmware blocks Win+L and Win+U on a host detected as Windows while any
GUI Mod-Tap is held. This includes `LGUI_T(KC_A)`, `RGUI_T(KC_SCLN)` and GUI
Mod-Taps assigned to other keys using Vial. Both left and right GUI work.
L/U may be plain keys or the tap side of Mod-Tap/Layer-Tap keys; their hold
functions are preserved. If L/U is already down, it is released before the GUI
hold is sent. Blocked presses are discarded and releases are always processed.

Dedicated Win keys retain Win+L and Win+U when no GUI Mod-Tap is held. Other
GUI shortcuts, Mod-Tap taps and Linux/unknown-host behavior are unchanged.
OS detection is a USB initialization heuristic; verify it on each host,
especially when using a USB switch or KVM. GUI itself remains available, so
Windows may still open Start when it is released.

The two rules are fixed in `shortcut_blocking.c`. There is no dynamic rule
editor protocol, custom rule table, or reserved shortcut EEPROM area. Existing
Pipette shortcut rules are ignored; its dynamic shortcut editor will report
this firmware as unsupported. Normal Vial configuration remains available.

## Vial configuration

The firmware uses the regular keymap defaults in `keymap.c`. Import your
`.vil` backup through Vial to restore personal layers, macros, combos and QMK
settings. Backups are not converted to C or automatically imported at startup.

Rotary encoders and their maps remain disabled. Vial shows the four EX
positions as ordinary keys, without rotary controls.

## Thumb tap-hold behavior

The six thumb positions (rows 3 and 7, columns 3 through 5) are exempt from
Chordal Hold's same-hand restriction and always enable Hold On Other Key Press.
Other positions continue to follow Vial's settings. Keep Chordal Hold enabled
and Hold On Other Key Press disabled in Vial to apply the latter only to thumbs.
The Chordal Hold exemption also applies when a thumb is the other key in a chord.
Pressing a thumb LT alone still waits for the tapping term; Quick Tap Term
continues to apply to repeated taps.

This requires the weak `get_hold_on_other_key_press` definition in
`quantum/qmk_settings.c`, included in this fork. Retain that change when merging
upstream updates.

## Build

From the repository root in QMK MSYS:

```sh
git submodule update --init --recursive
qmk compile -kb crkbd/rev4_1/standard -km vial
```

Output: `crkbd_rev4_1_standard_vial.uf2` in the repository root
(also under `.build/`). Keep a Vial layout backup before flashing.
The C keymap contains defaults, not your personal Vial EEPROM configuration;
import your `.vil` backup if needed.

## Regression tests

These tests use QMK's real tap/hold processing and inspect the resulting host
keyboard reports. Only the host OS result is stubbed; USB OS detection and real
hardware still need to be checked on each host.

From the repository root in QMK MSYS:

```sh
bash tests/windows_gui_l/run.sh
```
