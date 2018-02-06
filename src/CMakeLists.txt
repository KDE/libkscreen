include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES})

add_subdirectory(backendlauncher)
add_subdirectory(doctor)
set(libkscreen_SRCS
    abstractbackend.cpp
    backendmanager.cpp
    config.cpp
    configoperation.cpp
    getconfigoperation.cpp
    setconfigoperation.cpp
    configmonitor.cpp
    configserializer.cpp
    screen.cpp
    output.cpp
    edid.cpp
    mode.cpp
    debug_p.cpp
    log.cpp
)

qt5_add_dbus_interface(libkscreen_SRCS ${CMAKE_SOURCE_DIR}/interfaces/org.kde.KScreen.Backend.xml backendinterface)

add_library(KF5Screen SHARED ${libkscreen_SRCS})
generate_export_header(KF5Screen BASE_NAME KScreen)

target_link_libraries(KF5Screen
    PUBLIC
    Qt5::Core
    Qt5::Gui
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

install(TARGETS KF5Screen EXPORT KF5ScreenTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
add_library(KF5::Screen ALIAS KF5Screen)

ecm_generate_headers(KScreen_HEADERS
    HEADER_NAMES
        Log
        Mode
        Output
        EDID
        Screen
        Config
        ConfigMonitor
        ConfigOperation
        GetConfigOperation
        SetConfigOperation
        Types
    PREFIX KScreen
    REQUIRED_HEADERS KScreen_REQ_HEADERS
)

install(FILES ${KScreen_HEADERS}
        DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KScreen/KScreen
        COMPONENT Devel)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kscreen_export.h
              backendmanager_p.h # needed for unit-tests in KScreen
              ${KScreen_REQ_HEADERS}
        DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KScreen/kscreen)

if(NOT WIN32)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kscreen2.pc.in ${CMAKE_CURRENT_BINARY_DIR}/kscreen2.pc @ONLY)
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kscreen2.pc DESTINATION ${KDE_INSTALL_LIBDIR}/pkgconfig)
endif(NOT WIN32)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KScreen LIB_NAME KF5Screen DEPS "core" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
