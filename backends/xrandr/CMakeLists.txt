include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BUILD_DIR}
)

set(xrandr_SRCS
    xrandr.cpp
    xrandrconfig.cpp
    xrandrcrtc.cpp
    xrandroutput.cpp
    xrandrmode.cpp
    xrandrscreen.cpp
    ../xcbwrapper.cpp
    ../xcbeventlistener.cpp
)

add_library(KSC_XRandR MODULE ${xrandr_SRCS})

set_target_properties(KSC_XRandR PROPERTIES PREFIX "")
target_link_libraries(KSC_XRandR Qt5::Core
                                 Qt5::Gui
                                 Qt5::X11Extras
                                 ${XCB_LIBRARIES}
                                 KF5::Screen
)

install(TARGETS KSC_XRandR DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kscreen/)
