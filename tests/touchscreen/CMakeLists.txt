find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE REQUIRED
                        PURPOSE "Required for building the X11 based workspace"
                      )

add_executable(kscreen-touchscreen
                    main.cpp
                    tstool.cpp
                    xtouchscreen.cpp
                    )

target_link_libraries(kscreen-touchscreen
                      Qt5::Core
                      Qt5::Gui
                      KF5::Screen
                      ${X11_X11_LIB}
                      ${X11_Xinput_LIB}
)
# install(TARGETS kscreen-touchscreen
#         DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS}
# )
