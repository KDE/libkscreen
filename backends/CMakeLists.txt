add_subdirectory(fake)
add_subdirectory(qscreen)

if(ENABLE_WAYLAND)
    message(STATUS "Will build Wayland backend")
    add_subdirectory(wayland)
else()
    message(STATUS "Not building Wayland backend")
endif()

add_subdirectory(xrandr)
add_subdirectory(xrandr1.1)
