include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES})

set(libkscreen_SRCS
    backendloader.cpp
    config.cpp
    configmonitor.cpp
    screen.cpp
    output.cpp
    edid.cpp
    mode.cpp
)

qt4_automoc(${libkscreen_SRCS})

add_library(kscreen SHARED ${libkscreen_SRCS})

target_link_libraries(kscreen ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${KDE4_KDECORE_LIBS} ${QJSON_LIBRARIES})

set_target_properties(kscreen PROPERTIES
    VERSION ${libkscreen_VERSION}
    SOVERSION ${libkscreen_VERSION_MAJOR}
)

install(TARGETS kscreen EXPORT kscreenLibraryTargets  ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES kscreen_export.h
            mode.h
            output.h
            edid.h
            screen.h
            config.h
            configmonitor.h
            DESTINATION include/kscreen)

if(NOT WIN32)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kscreen.pc.in ${CMAKE_CURRENT_BINARY_DIR}/kscreen.pc @ONLY)
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kscreen.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif(NOT WIN32)