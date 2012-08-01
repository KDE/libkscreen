include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_BUILD_DIR} ${QT_INCLUDES})

set(xrandr_SRCS
    xrandr.cpp
)

automoc4_add_library(KSC_XRandR MODULE ${xrandr_SRCS})

set_target_properties(KSC_XRandR PROPERTIES PREFIX "")
target_link_libraries(KSC_XRandR ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QJSON_LIBRARIES} kscreen)

install(TARGETS KSC_XRandR DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/kscreen/)


