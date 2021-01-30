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
    ../utils.cpp
)

add_library(KSC_XRandR MODULE ${xrandr_SRCS})

set_target_properties(KSC_XRandR PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf5/kscreen")
set_target_properties(KSC_XRandR PROPERTIES PREFIX "")
target_link_libraries(KSC_XRandR Qt::Core
                                 Qt::Gui
                                 Qt::X11Extras
                                 ${XCB_LIBRARIES}
                                 KF5::Screen
)

install(TARGETS KSC_XRandR DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kscreen/)
