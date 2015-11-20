add_subdirectory(fake)
add_subdirectory(qscreen)

if(ENABLE_WAYLAND)
    message(STATUS "Will build Wayland backend")
    add_subdirectory(kwayland)
else()
    message(STATUS "Not building Wayland backend")
endif()

# There's no XCB_RANDR_FOUND variable defined, so we have to check manually
if(NOT ${XCB_RANDR_INCLUDE_DIR} STREQUAL "XCB_RANDR_INCLUDE_DIR-NOTFOUND")
    message(STATUS "Will build xrandr backend")
    add_subdirectory(xrandr)
    add_subdirectory(xrandr1.1)
else()
    message(STATUS "Not building xrandr backend, no XCB_RANDR found")
endif()
