include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BUILD_DIR}
)

set(qscreen_SRCS
    qscreenbackend.cpp
)

add_library(KSC_qscreen MODULE ${qscreen_SRCS})

set_target_properties(KSC_qscreen PROPERTIES PREFIX "")
target_link_libraries(KSC_qscreen Qt5::Core
                                 Qt5::Gui
                                 Qt5::X11Extras
                                 KF5::Screen
)

install(TARGETS KSC_qscreen DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kscreen/)
