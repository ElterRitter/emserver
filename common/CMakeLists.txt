cmake_minimum_required(VERSION 3.14)

project(TransportCommon LANGUAGES CXX)

if(NOT TARGET Boost::thread OR NOT TARGET Boost::system OR NOT TARGET Boost::regex)
    find_package(Boost COMPONENTS thread system regex REQUIRED)
endif()

set(SOURCES src/baseprotocol.cpp
            src/commonprocessor.cpp
            src/tcpsession.cpp
            src/processoremserver.cpp
            src/bufferprocessor.cpp
            )

set(HEADERS hdr/baseprotocol.h
            hdr/commonprocessor.h
            hdr/tcpsession.h
            hdr/ispecificprocessor.h
            hdr/processoremserver.h
            hdr/bufferprocessor.h
            )

option(${PROJECT_NAME}_USE_TESTING "Use unit tests in ${PROJECT_NAME}" OFF)
if(${${PROJECT_NAME}_USE_TESTING})
    add_subdirectory(tests)
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

source_group("Sources" FILES ${SOURCES})
source_group("Headers" FILES ${HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/hdr)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14
                                                 CXX_STANDARD_REQUIRED ON
                                                 POSITION_INDEPENDENT_CODE ON)


target_compile_definitions(${PROJECT_NAME} PUBLIC -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DOS_WIN -D_WIN32_WINNT=0x0601 )
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DBOOST_LIB_TOOLSET="vc141" )
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC SmLog Protocol Boost::thread Boost::system Boost::regex Boost::disable_autolinking)
