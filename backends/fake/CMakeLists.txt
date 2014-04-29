include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BUILD_DIR}
)

set(fake_SRCS
    fake.cpp
    parser.cpp
)

add_library(KSC_Fake MODULE ${fake_SRCS})

set_target_properties(KSC_Fake PROPERTIES PREFIX "")
target_link_libraries(KSC_Fake Qt5::Core
                               kscreen
)

install(TARGETS KSC_Fake DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kscreen/)


