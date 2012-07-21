include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES})

set(libkscreen_SRCS
    kscreen.cpp
)

QT4_AUTOMOC(${libkscreen_SRCS})

add_library(kscreen SHARED ${libkscreen_SRCS})

target_link_libraries(kscreen ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})

set_target_properties(kscreen PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS kscreen ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES kscreen_export.h
            kscreen.h DESTINATION include/kscreen)

if(NOT WIN32)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kscreen.pc.in ${CMAKE_CURRENT_BINARY_DIR}/kscreen.pc @ONLY)
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kscreen.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif(NOT WIN32)
