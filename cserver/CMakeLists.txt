cmake_minimum_required(VERSION 3.15)

if(NOT EmSMp_PROJECT_VERSION)
    set(CSERVER_VERSION 0.1)
else()
    set(CSERVER_VERSION ${EmSMp_PROJECT_VERSION})
endif()

project(CServer VERSION ${CSERVER_VERSION})

option(PACKAGING_${PROJECT_NAME} "Create package of ${PROJECT_NAME}" OFF)
option(${PROJECT_NAME}_USE_COUNTERS "Use counters of client's callbacks" OFF)

set(${PROJECT_NAME}_API_HEADERS cserver_global.h
                                api_server.h
                                api_common.h
                                api_capabilities.h
                                api_sensors.h)

set(HEADERS cserver.h)

set(SOURCES cserver.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${${PROJECT_NAME}_API_HEADERS})

source_group("Sources" FILES ${SOURCES})
source_group("Headers" FILES ${HEADERS})
source_group("API Headers" FILES ${${PROJECT_NAME}_API_HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

target_compile_definitions(${PROJECT_NAME} PUBLIC -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DCSERVER_LIBRARY)
if(${PROJECT_NAME}_USE_COUNTERS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DUSE_COUNTERS_OF_CLIENTCALLBACK)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} PUBLIC Server Protocol TransportCommon SmLog)

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

set(${PROJECT_NAME}_BIN_DESTINATION "bin")
set(${PROJECT_NAME}_LIB_DESTINATION "lib")
set(${PROJECT_NAME}_INC_DESTINATION "include")

if(PACKAGING_${PROJECT_NAME})
    add_subdirectory(package)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${PROJECT_NAME}/${${PROJECT_NAME}_BIN_DESTINATION}" COMPONENT bin
                                LIBRARY DESTINATION "${PROJECT_NAME}/${${PROJECT_NAME}_BIN_DESTINATION}" COMPONENT bin
                                ARCHIVE DESTINATION "${PROJECT_NAME}/${${PROJECT_NAME}_LIB_DESTINATION}" COMPONENT lib)


install(FILES ${${PROJECT_NAME}_API_HEADERS} DESTINATION "${PROJECT_NAME}/${${PROJECT_NAME}_INC_DESTINATION}" COMPONENT dev)
