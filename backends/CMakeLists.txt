add_subdirectory(fake)
add_subdirectory(kwayland)

if(${XCB_RANDR_FOUND})
    message(STATUS "Will build xrandr backend.")
    add_subdirectory(xrandr)
else()
    message(STATUS "Not building xrandr backend, no XCB_RANDR_FOUND set.")
endif()
