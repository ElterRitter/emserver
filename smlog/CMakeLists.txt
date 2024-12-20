cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(SmLog LANGUAGES CXX VERSION 0.0.1)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules")

option(USE_EXTERNAL_LOGGING_LIB "Use external logging library" OFF)


add_library(${PROJECT_NAME} STATIC)

if(USE_EXTERNAL_LOGGING_LIB)
    find_package(Log4cpp REQUIRED)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DUSE_LIB_LOGGING)
    target_link_libraries(${PROJECT_NAME} PUBLIC Log4cpp::log4cpp)
endif()

set(HEADERS "hdr/smlog.h")
set(SOURCES "src/smlog.cpp")

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} ${HEADERS})

source_group("Sources" FILES ${SOURCES})
source_group("Headers" FILES ${HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/hdr)
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DOS_WIN)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_definitions(${PROJECT_NAME} PRIVATE -DDEBUG_BUILD)
    endif()
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC -DLOG4CPP_HAVE_SSTREAM)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

