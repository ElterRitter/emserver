cmake_minimum_required(VERSION 3.15)

project(native_cserver VERSION 0.1)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

set(SOURCES main.cpp)

set(HEADERS )

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

source_group("Sources" FILES ${SOURCES})
source_group("Headers" FILES ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE CServer Qt5::Core)
