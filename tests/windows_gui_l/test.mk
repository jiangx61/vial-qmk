# Prepared and invoked by run.sh.
# LTO also lets MinGW resolve QMK's weak function defaults across translation units.
LTO_ENABLE = yes
SRC += $(TEST_PATH)/shortcut_blocking.c
