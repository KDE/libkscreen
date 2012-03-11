include_directories(${QT_INCLUDES} ${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${X11_Xrandr_INCLUDE_PATH})

set(testApp_SRCS
    ../src/lib/xrandr.cpp
    ../src/lib/screen.cpp
    ../src/lib/crtc.cpp
    ../src/lib/output.cpp
    ../src/lib/mode.cpp
    testapp.cpp
)

kde4_add_executable(testapp ${testApp_SRCS})

target_link_libraries(testapp
                      ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} ${X11_Xrandr_LIB} ${X11_LIBRARIES})
