add_subdirectory(doctor)
add_subdirectory(libdpms)
set(libkscreen_SRCS
    abstractbackend.cpp abstractbackend.h
    backendmanager.cpp
    config.cpp
    configoperation.cpp configoperation.h
    getconfigoperation.cpp getconfigoperation.h
    setconfigoperation.cpp setconfigoperation.h
    configmonitor.cpp configmonitor.h
    configserializer.cpp
    screen.cpp
    output.cpp
    edid.cpp
    mode.cpp

    ../backends/kwayland/waylandbackend.cpp ../backends/kwayland/waylandbackend.h
    ../backends/kwayland/waylandconfig.cpp ../backends/kwayland/waylandconfig.h
    ../backends/kwayland/waylandoutputdevice.cpp ../backends/kwayland/waylandoutputdevice.h
    ../backends/kwayland/waylandoutputmanagement.cpp ../backends/kwayland/waylandoutputmanagement.h
    ../backends/kwayland/waylandoutputdevicemode.cpp ../backends/kwayland/waylandoutputdevicemode.h
    ../backends/kwayland/waylandscreen.cpp ../backends/kwayland/waylandscreen.h
    ../backends/utils.cpp ../backends/utils.h

    ../backends/fake/fake.cpp
    ../backends/fake/parser.cpp
    ../backends/fake/fake.h
    ../backends/fake/parser.h
)

qt_add_dbus_interface(libkscreen_SRCS ../backends/kwayland/org.kde.KWin.TabletModeManager.xml tabletmodemanager_interface)

ecm_qt_declare_logging_category(libkscreen_SRCS
                                HEADER kscreen_kwayland_logging.h
                                IDENTIFIER KSCREEN_WAYLAND
                                CATEGORY_NAME kscreen.kwayland
				DESCRIPTION "kscreen wayland"
				EXPORT KSCREEN
)

ecm_qt_declare_logging_category(libkscreen_SRCS
                                HEADER kscreen_debug.h
                                IDENTIFIER KSCREEN 
                                CATEGORY_NAME org.kde.kscreen
				DESCRIPTION "kscreen"
				EXPORT KSCREEN
)

ecm_qt_declare_logging_category(libkscreen_SRCS
                                HEADER kscreen_debug_edid.h
                                IDENTIFIER KSCREEN_EDID
                                CATEGORY_NAME org.kde.kscreen.edid
				DESCRIPTION "Kscreen edid"
				EXPORT KSCREEN
)


add_library(KF6Screen SHARED ${libkscreen_SRCS})
generate_export_header(KF6Screen BASE_NAME KScreen)

ecm_create_qm_loader(KF6Screen libkscreen6_qt)

qt6_generate_wayland_protocol_client_sources(KF6Screen
    PRIVATE_CODE
    FILES
        ${PLASMA_WAYLAND_PROTOCOLS_DIR}/kde-output-device-v2.xml
        ${PLASMA_WAYLAND_PROTOCOLS_DIR}/kde-output-management-v2.xml
)

target_sources(KF6Screen PRIVATE ${wayland_SRCS})

target_link_libraries(KF6Screen
    PUBLIC
    Qt::Core
    Qt::Gui
    PRIVATE
    Qt::DBus
    Qt::GuiPrivate # for QX11Info
    Qt::WaylandClient
    Wayland::Client
)

set_target_properties(KF6Screen PROPERTIES
    VERSION "${KSCREEN_VERSION}"
    SOVERSION "${KSCREEN_SOVERSION}"
    EXPORT_NAME Screen
)

target_include_directories(KF6Screen PUBLIC "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KScreen>")

install(TARGETS KF6Screen EXPORT KF6ScreenTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
add_library(KF6::Screen ALIAS KF6Screen)

ecm_generate_headers(KScreen_HEADERS
    HEADER_NAMES
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
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KScreen/KScreen
        COMPONENT Devel)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kscreen_export.h
              backendmanager_p.h # needed for unit-tests in KScreen
              ${KScreen_REQ_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KScreen/kscreen)

if(NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME KF6Screen
        LIB_NAME KF6Screen
        INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KScreen
        DEPS Qt6Core Qt6Gui
        DESCRIPTION "Qt Based library to manage screens with backends (xrandr, whatevercomesnext)"
        INSTALL
     )
endif(NOT WIN32)

if(BUILD_QCH)
    ecm_add_qch(
        KF6Screen_QCH
        NAME KScreen
        BASE_NAME KF6Screen
        VERSION ${PROJECT_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KScreen_REQ_HEADERS}
        # MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
            Qt6Gui_QCH
        BLANK_MACROS
            KSCREEN_EXPORT
            KSCREEN_DEPRECATED
            KSCREEN_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
