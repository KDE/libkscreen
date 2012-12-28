project(libkscreen)
set(libkscreen_VERSION_MAJOR 0)
set(libkscreen_VERSION_MINOR 9)
set(libkscreen_VERSION_RELEASE 0)
set(libkscreen_VERSION "0.9.0")

cmake_minimum_required(VERSION 2.8.0)

if(POLICY CMP0017)
  cmake_policy(SET CMP0017 NEW)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
enable_testing()
find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)
find_package(QJSON 0.8 REQUIRED)
include(KDE4Defaults)

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-fvisibility=hidden _HAVE_VISIBILITY)
if (_HAVE_VISIBILITY AND NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

  check_cxx_compiler_flag(-fvisibility-inlines-hidden _HAVE_VISIBILITY_INLINES)
  if (_HAVE_VISIBILITY_INLINES)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
  endif (_HAVE_VISIBILITY_INLINES)
endif (_HAVE_VISIBILITY AND NOT WIN32)

#
### lib64/32 support kde like
#
set(LIB_SUFFIX "" CACHE STRING "Define suffix for lib directory (32/64)")

set(BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})

set(INSTALL_TARGETS_DEFAULT_ARGS  RUNTIME DESTINATION "${BIN_INSTALL_DIR}"
                                  LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                                  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}" COMPONENT Devel )

add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(backends)
add_subdirectory(tests)

configure_file(${libkscreen_SOURCE_DIR}/LibKScreenConfig.cmake.in
               ${libkscreen_BINARY_DIR}/LibKScreenConfig.cmake
               @ONLY
)
# this file is used by to check if the installed version can be used.
macro_write_basic_cmake_version_file(${libkscreen_BINARY_DIR}/LibKScreenConfigVersion.cmake
                                     ${libkscreen_VERSION_MAJOR} ${libkscreen_VERSION_MINOR} ${libkscreen_VERSION_RELEASE})

install(FILES ${libkscreen_BINARY_DIR}/LibKScreenConfig.cmake ${libkscreen_BINARY_DIR}/LibKScreenConfigVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/cmake/LibKScreen)

# Install the file with the exported targets
install(EXPORT kscreenLibraryTargets
        DESTINATION ${LIB_INSTALL_DIR}/cmake/LibKScreen
        FILE LibKScreenTargetsWithPrefix.cmake)



############################################ apidox ############################################

option(LIBKSCREEN_BUILD_API_DOCS "Build libkscreen API documentation")

if(LIBKSCREEN_BUILD_API_DOCS)
  find_package(Doxygen)

  if(DOXYGEN_EXECUTABLE)
    configure_file(${libkscreen_SOURCE_DIR}/.Doxyfile.cmake ${libkscreen_BINARY_DIR}/Doxyfile)

    if(EXISTS ${QT_DOC_DIR}/html)
      set(QTDOCS "${QT_DOC_DIR}/html")
    else(EXISTS ${QT_DOC_DIR}/html)
      set(QTDOCS "http://doc.qt.nokia.com/latest/")
    endif(EXISTS ${QT_DOC_DIR}/html)

    add_custom_target(
      apidox ALL
      COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
      COMMAND doc/html/installdox -l qt4.tag@${QTDOCS} doc/html/*.html)
  endif(DOXYGEN_EXECUTABLE)
endif(LIBKSCREEN_BUILD_API_DOCS)
