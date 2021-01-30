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

set_target_properties(KSC_QScreen PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf5/kscreen")
set_target_properties(KSC_QScreen PROPERTIES PREFIX "")
target_link_libraries(KSC_QScreen Qt::Core
                                 Qt::Gui
                                 Qt::X11Extras
                                 KF5::Screen
)

install(TARGETS KSC_QScreen DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kscreen/)
