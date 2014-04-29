find_package(XCB REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src
                    ${KDE4_INCLUDES}
                    ${CMAKE_BUILD_DIR}
                    ${QT_INCLUDES}
                    ${X11_Xrandr_INCLUDE_PATH}
)

set(xrandr11_SRCS
    xrandr11.cpp
    ../xrandr/xrandrxcbhelper.cpp
)

add_library(KSC_XRandR11 MODULE ${xrandr11_SRCS})

set_target_properties(KSC_XRandR11 PROPERTIES PREFIX "")
target_link_libraries(KSC_XRandR11 Qt5::Core
                                   Qt5::Gui
                                   Qt5::X11Extras
                                   ${X11_LIBRARIES}
                                   ${X11_Xrandr_LIB}
                                   ${XCB_LIBRARIES}
                                   KF5::Screen
)

install(TARGETS KSC_XRandR11 DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kscreen/)
