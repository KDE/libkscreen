add_definitions(-DTEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/configs/")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

macro(KSCREEN_ADD_TEST)
    foreach(_testname ${ARGN})
        set(test_SRCS ${_testname}.cpp ${KSCREEN_WAYLAND_SRCS})
        qt5_add_dbus_interface(test_SRCS ${CMAKE_SOURCE_DIR}/interfaces/org.kde.KScreen.FakeBackend.xml fakebackendinterface)
        add_executable(${_testname} ${test_SRCS})
        target_link_libraries(${_testname} Qt5::Core Qt5::Gui Qt5::Test Qt5::DBus KF5::Screen ${KSCREEN_WAYLAND_LIBS})
        add_test(NAME kscreen-${_testname}
                 COMMAND dbus-launch $<TARGET_FILE:${_testname}>
        )
        ecm_mark_as_test(${_testname})
    endforeach(_testname)
endmacro(KSCREEN_ADD_TEST)

kscreen_add_test(testscreenconfig)
kscreen_add_test(testqscreenbackend)
kscreen_add_test(testconfigserializer)
kscreen_add_test(testconfigmonitor)
kscreen_add_test(testinprocess)

if(ENABLE_WAYLAND AND KF5Config_FOUND)
    set(KSCREEN_WAYLAND_LIBS
        KF5::WaylandServer KF5::WaylandClient KF5::CoreAddons KF5::ConfigCore
    )

    # For WaylandConfigWriter and -Reader and TestServer
    set(KSCREEN_WAYLAND_SRCS
        ${CMAKE_SOURCE_DIR}/backends/wayland/waylandconfigwriter.cpp
        ${CMAKE_SOURCE_DIR}/backends/wayland/waylandconfigreader.cpp
        ${CMAKE_SOURCE_DIR}/tests/waylandtestserver.cpp
    )
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../backends/wayland)

    kscreen_add_test(testwaylandbackend)
    kscreen_add_test(testwlsetup)
    kscreen_add_test(testwlwrite)
    kscreen_add_test(testwloutputs)

    set(KSCREEN_WAYLAND_LIBS "")
    set(KSCREEN_WAYLAND_SRCS "")
endif()

if (ENABLE_XRANDR_TESTS)
    kscreen_add_test(textxrandr)
endif()
