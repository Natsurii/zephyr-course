# SPDX-License-Identifier: Apache-2.0

# Board bring-up hook: prints "Board Initialized" before the application
# entry point runs.
zephyr_library()
zephyr_library_sources(board_init.c)