add_definitions(-DTEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/configs/")

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/tests/kwayland/)

macro(KSCREEN_ADD_TEST)
    foreach(_testname ${ARGN})
        set(test_SRCS ${_testname}.cpp ${KSCREEN_WAYLAND_SRCS})
        qt_add_dbus_interface(test_SRCS ${CMAKE_SOURCE_DIR}/interfaces/org.kde.KScreen.FakeBackend.xml fakebackendinterface)
        add_executable(${_testname} ${test_SRCS})
        target_link_libraries(${_testname} Qt::Core Qt::Gui Qt::Test Qt::DBus KF5::Screen ${KSCREEN_WAYLAND_LIBS})
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
kscreen_add_test(testbackendloader)
kscreen_add_test(testlog)
kscreen_add_test(testmodelistchange)
kscreen_add_test(testedid)

set(KSCREEN_WAYLAND_LIBS
    KF5::WaylandServer KF5::WaylandClient
)

# For WaylandConfigReader and TestServer
set(KSCREEN_WAYLAND_SRCS
    ${CMAKE_SOURCE_DIR}/tests/kwayland/waylandconfigreader.cpp
    ${CMAKE_SOURCE_DIR}/tests/kwayland/waylandtestserver.cpp
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../backends/kwayland)

kscreen_add_test(testkwaylandbackend)
kscreen_add_test(testkwaylandconfig)
kscreen_add_test(testkwaylanddpms)

set(KSCREEN_WAYLAND_LIBS "")
set(KSCREEN_WAYLAND_SRCS "")


if (ENABLE_XRANDR_TESTS)
    kscreen_add_test(textxrandr)
endif()
