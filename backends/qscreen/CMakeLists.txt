include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BUILD_DIR}
)

set(qscreen_SRCS
    qscreenbackend.cpp
    qscreenconfig.cpp
    qscreenscreen.cpp
    qscreenoutput.cpp
)

add_library(KSC_QScreen MODULE ${qscreen_SRCS})

set_target_properties(KSC_QScreen PROPERTIES PREFIX "")
target_link_libraries(KSC_QScreen Qt5::Core
                                 Qt5::Gui
                                 Qt5::X11Extras
                                 KF5::Screen
)

install(TARGETS KSC_QScreen DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kscreen/)
