cmake_minimum_required(VERSION 3.15)

project(Server VERSION 0.1)

#set(Boost_USE_STATIC_LIBS ON)
if(NOT TARGET boost::thread OR NOT TARGET Boost::system OR NOT TARGET Boost::regex)
    find_package(Boost REQUIRED COMPONENTS thread system regex)
endif()

set(HEADERS hdr/transport.h

)

set(SOURCES src/transport.cpp

)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

source_group("Sources" FILES ${SOURCES})
source_group("Headers" FILES ${HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/hdr)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DOS_WIN -D_WIN32_WINNT=0x0601)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DBOOST_LIB_TOOLSET=vc1429)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} PUBLIC Protocol TransportCommon SmLog Boost::thread Boost::system Boost::regex Boost::disable_autolinking)
