cmake_minimum_required(VERSION 3.20.0)

# Expose the LED sensor driver module (drivers, binding, Kconfig).
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/modules/led_sensor)

find_package(Zephyr REQUIRED)
project(my_zephyr_app)

target_sources(app PRIVATE src/main.cpp)