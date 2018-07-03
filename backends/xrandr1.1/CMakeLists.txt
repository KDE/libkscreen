find_package(XCB REQUIRED COMPONENTS XCB RANDR)

include_directories(${CMAKE_SOURCE_DIR}/src
                    ${KDE4_INCLUDES}
                    ${CMAKE_BUILD_DIR}
                    ${QT_INCLUDES}
)

set(xrandr11_SRCS
    xrandr11.cpp
    ../xcbeventlistener.cpp
    ../xcbwrapper.cpp
)

add_library(KSC_XRandR11 MODULE ${xrandr11_SRCS})

set_target_properties(KSC_XRandR11 PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf5/kscreen")
set_target_properties(KSC_XRandR11 PROPERTIES PREFIX "")
target_link_libraries(KSC_XRandR11 Qt5::Core
                                   Qt5::Gui
                                   Qt5::X11Extras
                                   ${XCB_LIBRARIES}
                                   KF5::Screen
)

install(TARGETS KSC_XRandR11 DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kscreen/)
