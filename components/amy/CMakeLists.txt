# apply patches to vendor branch

# this doesn't work because of esp-idf preprocessing pass
# set(AMY_SRC  "${CMAKE_CURRENT_SOURCE_DIR}/upstream")
# set(AMY_PATCHES
#     "${CMAKE_CURRENT_SOURCE_DIR}/patches/0001-i2s-task-signature.patch"
# )

set(AMY_SRC     "${COMPONENT_DIR}/upstream")
set(AMY_PATCHES
    "${COMPONENT_DIR}/patches/0001-i2s-task-signature.patch"
    "${COMPONENT_DIR}/patches/0002-midi-task-signature.patch"
    "${COMPONENT_DIR}/patches/0003-task-priority-off-by-one.patch"
)

find_program(GIT_EXECUTABLE git REQUIRED)

foreach(patch ${AMY_PATCHES})
    # --check --reverse succeeds if the patch is ALREADY applied -> skip it
    execute_process(
        COMMAND ${GIT_EXECUTABLE} apply --check --reverse "${patch}"
        WORKING_DIRECTORY "${AMY_SRC}"
        RESULT_VARIABLE already_applied
        ERROR_QUIET OUTPUT_QUIET
    )

    message(STATUS "AMY patch stdout: ${patch_out}")
    message(STATUS "AMY patch stderr: ${patch_err}")
    message(STATUS "AMY patch WORKING_DIRECTORY was: ${AMY_SRC}")

    if(NOT already_applied EQUAL 0)
        message(STATUS "AMY: applying patch ${patch}")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} apply "${patch}"
            WORKING_DIRECTORY "${AMY_SRC}"
            RESULT_VARIABLE patch_result
            OUTPUT_VARIABLE patch_out
            ERROR_VARIABLE  patch_err
        )
        if(NOT patch_result EQUAL 0)
            message(FATAL_ERROR "AMY: failed to apply ${patch}")
        endif()
    else()
        message(STATUS "AMY: patch already applied, skipping ${patch}")
    endif()
endforeach()



file(GLOB AMY_SOURCES "upstream/src/*.c")

list(REMOVE_ITEM AMY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/upstream/src/amy-example.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/upstream/src/libminiaudio-audio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/upstream/src/examples.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/upstream/src/usb.c"

)

idf_component_register(
    SRCS ${AMY_SOURCES}
    INCLUDE_DIRS "upstream/src"
    REQUIRES
        esp_driver_uart      # driver/uart.h   — MIDI UART
        esp_driver_i2s       # driver/i2s_*.h  — audio output
        esp_driver_gpio      # driver/gpio.h
        esp_timer            # esp_timer.h
        esp_driver_i2c       # driver/i2c_*.h  — if AMY touches I²C
)
