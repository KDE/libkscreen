add_definitions(-DTEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/configs/")


macro(KSCREEN_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} Qt5::Core Qt5::Gui Qt5::Test KF5::Screen ${KSCREEN_WAYLAND_LIBS})
        add_test(kscreen-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
    endforeach(_testname)
endmacro(KSCREEN_TESTS)

KSCREEN_TESTS(testscreenconfig )
KSCREEN_TESTS(testqscreenbackend )

if(ENABLE_WAYLAND)
    set(KSCREEN_WAYLAND_LIBS KF5::WaylandServer KF5::WaylandClient)
    KSCREEN_TESTS(testwaylandbackend testwaylandbackendconfigs)
    set(KSCREEN_WAYLAND_LIBS "")
endif()

if (ENABLE_XRANDR_TESTS)
    KSCREEN_TESTS(testxrandr )
endif()
