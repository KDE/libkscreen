include_directories(${CMAKE_SOURCE_DIR}/src ${KDE4_INCLUDES} ${CMAKE_BUILD_DIR} ${QT_INCLUDES} ${X11_Xrandr_INCLUDE_PATH} )

set(xrandr_SRCS
    xrandr.cpp
    xrandrx11helper.cpp
    xrandrconfig.cpp
    xrandroutput.cpp
    xrandrmode.cpp
    xrandrscreen.cpp
)

automoc4_add_library(KSC_XRandR MODULE ${xrandr_SRCS})

set_target_properties(KSC_XRandR PROPERTIES PREFIX "")
target_link_libraries(KSC_XRandR ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}  ${QT_QTDBUS_LIBRARY} ${QJSON_LIBRARIES} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${X11_Xrandr_LIB} ${X11_LIBRARIES} kscreen)

install(TARGETS KSC_XRandR DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/kscreen/)