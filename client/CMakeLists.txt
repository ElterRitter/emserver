cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(Client LANGUAGES CXX VERSION 0.0.1)

if(BUILD_SHARED_LIBS)
    set(${PROJECT_NAME}_BUILD_SHARED_DEFAULT ON)
else()
    set(${PROJECT_NAME}_BUILD_SHARED_DEFAULT OFF)
endif()

option(${PROJECT_NAME}_BUILD_SHARED "Build ${PROJECT_NAME} as a shared library" ${${PROJECT_NAME}_BUILD_SHARED_DEFAULT})
if(${PROJECT_NAME}_BUILD_SHARED)
    message(STATUS "Building library ${PROJECT_NAME} as SHARED library")
    set(${PROJECT_NAME}_LIB_TYPE "SHARED")
else()
    message(STATUS "Building library ${PROJECT_NAME} as STATIC library")
    set(${PROJECT_NAME}_LIB_TYPE "STATIC")
endif()

if(NOT TARGET Boost::system OR NOT TARGET Boost::thread)
    find_package(Boost REQUIRED COMPONENTS thread system )
endif()

if(BUILD_DOCUMENTATION)
    find_package(Doxygen QUIET)
    if(Doxygen_FOUND)
        set(DOC_CLIENT_VERSION "${EmSMp_PROJECT_VERSION_MAJOR}.${EmSMp_PROJECT_VERSION_MINOR}.${EmSMp_PROJECT_VERSION_REVISION}")
        set(doxyfile_in ${PROJECT_SOURCE_DIR}/doxy/doxyfile.in)
        set(doxyfile ${PROJECT_BINARY_DIR}/doxyfile)
        configure_file(${doxyfile_in} ${doxyfile} @ONLY)
        add_custom_target(Documentation_${PROJECT_NAME} COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM)
        message(STATUS "Target Documentation_${PROJECT_NAME} builds client library documentation")
    else()
        message(STATUS "Can't build client library documentation: doxygen not found")
    endif()
endif()

set(${PROJECT_NAME}_PUBLIC_HEADERS hdr/interop.h
                                   hdr/interop_sensors.h
                                   hdr/dataconverter.h
                                   hdr/emserverfacade.h
                                   hdr/client_global.h)

set(HEADERS hdr/transport_client.h
    hdr/emserveradapter.h)

set(SOURCES src/transport_client.cpp
            src/emserveradapter.cpp
            src/dataconverter.cpp
            src/emserverfacade.cpp
            src/interop_sensors.cpp)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_LIB_TYPE} ${SOURCES} ${HEADERS} ${${PROJECT_NAME}_PUBLIC_HEADERS})

source_group("Sources" FILES ${SOURCES})
source_group("Headers" FILES ${HEADERS})
source_group("API Headers" FILES ${${PROJECT_NAME}_PUBLIC_HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/hdr)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DOS_WIN -D_WIN32_WINNT=0x0601)
endif()

if(${PROJECT_NAME}_BUILD_SHARED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DCLIENT_LIBRARY)
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DCLIENT_BUILD_STATIC)
endif()

option(${PROJECT_NAME}_USE_CALLBACK_COUNTERS "${PROJECT_NAME} use a spectr callback counter" OFF)
if(${PROJECT_NAME}_USE_CALLBACK_COUNTERS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_CALLBACK_COUNTERS)
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC TransportCommon Protocol SmLog Boost::thread Boost::system)

set(BIN_DESTINATION "bin")
set(LIB_DESTINATION "lib")
set(INC_DESTINATION "include")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${PROJECT_NAME}/${BIN_DESTINATION}" COMPONENT bin
                                LIBRARY DESTINATION "${PROJECT_NAME}/${BIN_DESTINATION}" COMPONENT bin
                                ARCHIVE DESTINATION "${PROJECT_NAME}/${LIB_DESTINATION}" COMPONENT bin)

install(FILES ${${PROJECT_NAME}_PUBLIC_HEADERS} DESTINATION "${PROJECT_NAME}/${INC_DESTINATION}" COMPONENT devel)
