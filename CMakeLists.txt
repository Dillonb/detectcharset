cmake_minimum_required (VERSION 3.10)
project (detectcharset)

find_package(PkgConfig)
if (PkgConfig_FOUND)
    pkg_check_modules(CHARDET chardet)
endif ()

add_executable(detectcharset main.cpp)
target_include_directories(detectcharset PRIVATE ${CHARDET_INCLUDE_DIRS})
target_link_directories(detectcharset PRIVATE ${CHARDET_LIBRARY_DIRS})
target_link_libraries(detectcharset PRIVATE ${CHARDET_LIBRARIES})
install(TARGETS detectcharset DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)