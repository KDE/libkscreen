include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES})

set(libkscreen_SRCS
    backendloader.cpp
    config.cpp
    configmonitor.cpp
    screen.cpp
    output.cpp
    edid.cpp
    mode.cpp
    debug_p.cpp
)

add_library(KF5Screen SHARED ${libkscreen_SRCS})

target_link_libraries(KF5Screen PUBLIC Qt5::Core)

set_target_properties(KF5Screen PROPERTIES
    VERSION "${KSCREEN_VERSION_STRING}"
    SOVERSION "${KSCREEN_SOVERSION}"
    EXPORT_NAME Screen
)

install(TARGETS KF5Screen EXPORT KF5ScreenTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
add_library(KF5::Screen ALIAS KF5Screen)

install(FILES kscreen_export.h
            mode.h
            output.h
            edid.h
            screen.h
            config.h
            configmonitor.h
            DESTINATION ${INCLUDE_INSTALL_DIR}/kscreen)

if(NOT WIN32)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kscreen.pc.in ${CMAKE_CURRENT_BINARY_DIR}/kscreen.pc @ONLY)
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kscreen.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif(NOT WIN32)
