include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_BUILD_DIR} ${QT_INCLUDES})
include_directories(${QJSON_INCLUDE_DIR})

set(fake_SRCS
    fake.cpp
    parser.cpp
)

automoc4_add_library(KSC_Fake MODULE ${fake_SRCS})

set_target_properties(KSC_Fake PROPERTIES PREFIX "")
target_link_libraries(KSC_Fake ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QJSON_LIBRARIES} kscreen)

install(TARGETS KSC_Fake DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/kscreen/)


