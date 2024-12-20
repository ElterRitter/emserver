cmake_minimum_required(VERSION 3.15)

project(native_client VERSION 0.1)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(HEADERS clientwgt.h emserveradapterqt.h)

set(SOURCES main.cpp clientwgt.cpp emserveradapterqt.cpp)

set(FORMS clientwgt.ui)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${FORMS} )

source_group("Sources" FILES ${SOURCES})
source_group("Headers" FILES ${HEADERS})
source_group("Forms" FILES ${FORMS})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/hdr)
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DOS_WIN -D_WIN32_WINNT=0x0601)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DBOOST_LIB_TOOLSET=vc1429)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Client Qt5::Core Qt5::Gui Qt5::Widgets)
