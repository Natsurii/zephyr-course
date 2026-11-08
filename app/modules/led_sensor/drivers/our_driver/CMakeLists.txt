# SPDX-License-Identifier: Apache-2.0

if(CONFIG_OUR_DRIVER)
  zephyr_library()
  zephyr_library_sources(our_driver.c)

  # Shell front end is optional and only compiled when asked for.
  if(CONFIG_OUR_DRIVER_SHELL)
    zephyr_library_sources(our_driver_shell.c)
  endif()
endif()
