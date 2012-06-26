project(screenmanagement)

cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)
find_package(X11 REQUIRED)

add_subdirectory(src)
