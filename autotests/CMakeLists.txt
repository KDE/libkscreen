add_definitions(-DTEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/configs/")

macro(KSCREEN_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} Qt5::Core Qt5::Gui Qt5::Test KF5::Screen)
        add_test(kscreen-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
    endforeach(_testname)
endmacro(KSCREEN_TESTS)

KSCREEN_TESTS(testscreenconfig )

if (ENABLE_XRANDR_TESTS)
    KSCREEN_TESTS(testxrandr )
endif()
