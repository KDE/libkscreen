include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BUILD_DIR}
)

set(wayland_SRCS
    waylandbackend.cpp
    waylandconfig.cpp
    waylandconfigwriter.cpp
    waylandoutput.cpp
    waylandscreen.cpp
)

add_library(KSC_KWayland MODULE ${wayland_SRCS})

set_target_properties(KSC_KWayland PROPERTIES PREFIX "")
target_link_libraries(KSC_KWayland Qt5::Core
                                  Qt5::Gui
                                  Qt5::DBus
                                  Qt5::Test
                                  Wayland::Client
#                                   Wayland::Cursor
                                  KF5::Screen
                                  KF5::WaylandClient
)

install(TARGETS KSC_KWayland DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kscreen/)
