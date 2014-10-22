include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES})

add_subdirectory(backendlauncher)

set(libkscreen_SRCS
    abstractbackend.cpp
    backendmanager.cpp
    config.cpp
    configmonitor.cpp
    configserializer.cpp
    screen.cpp
    output.cpp
    edid.cpp
    mode.cpp
    debug_p.cpp
)

qt5_add_dbus_interface(libkscreen_SRCS ${CMAKE_SOURCE_DIR}/interfaces/org.kde.KScreen.Backend.xml backendinterface)

add_library(KF5Screen SHARED ${libkscreen_SRCS})

target_link_libraries(KF5Screen
    PUBLIC
    Qt5::Core
    PRIVATE
    Qt5::DBus
    Qt5::X11Extras
)

set_target_properties(KF5Screen PROPERTIES
    VERSION "${KSCREEN_VERSION_STRING}"
    SOVERSION "${KSCREEN_SOVERSION}"
    EXPORT_NAME Screen
)

target_include_directories(KF5Screen PUBLIC "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KScreen>")

install(TARGETS KF5Screen EXPORT KF5ScreenTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
add_library(KF5::Screen ALIAS KF5Screen)

ecm_generate_headers(KScreen_HEADERS
    HEADER_NAMES
        Mode
        Output
        EDID
        Screen
        Config
        ConfigMonitor
    PREFIX KScreen
    REQUIRED_HEADERS KScreen_REQ_HEADERS
)

install(FILES ${KScreen_HEADERS}
        DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KScreen/KScreen
        COMPONENT Devel)
install(FILES kscreen_export.h
            ${KScreen_REQ_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KScreen/kscreen)

if(NOT WIN32)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kscreen2.pc.in ${CMAKE_CURRENT_BINARY_DIR}/kscreen2.pc @ONLY)
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kscreen2.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif(NOT WIN32)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KScreen LIB_NAME KF5Screen DEPS "core" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
