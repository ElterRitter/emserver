cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(EmServerProxy LANGUAGES CXX VERSION 0.0.1)

# Generate project version
set(EmSMp_PROJECT_VERSION_MAJOR)
set(EmSMp_PROJECT_VERSION_MINOR)
set(EmSMp_PROJECT_VERSION_REVISION)

# find_package(Git QUIET)
# if(EXISTS ${GIT_EXECUTABLE})
#     message(STATUS "Generate project version/revision with ${GIT_EXECUTABLE}")
#     execute_process(COMMAND ${GIT_EXECUTABLE} "describe" "--long" WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} TIMEOUT 5 OUTPUT_VARIABLE STR_VERSION)
#     string(STRIP ${STR_VERSION} STR_VERSION)
#     string(REPLACE "." ";" STR_VERSION "${STR_VERSION}")
#     string(REPLACE "-" ";" STR_VERSION "${STR_VERSION}")

#     list(GET STR_VERSION 0 EmSMp_PROJECT_VERSION_MAJOR)
#     list(GET STR_VERSION 1 EmSMp_PROJECT_VERSION_MINOR)
#     list(GET STR_VERSION 2 EmSMp_PROJECT_VERSION_PATCH)
#     list(GET STR_VERSION 3 EmSMp_PROJECT_VERSION_REVISION)

#     set(EmSMp_PROJECT_VERSION "${EmSMp_PROJECT_VERSION_MAJOR}.${EmSMp_PROJECT_VERSION_MINOR}.${EmSMp_PROJECT_VERSION_PATCH}")
#     set(EmSMp_PROJECT_VERSION_FULL "${EmSMp_PROJECT_VERSION_MAJOR}.${CEmSMp_PROJECT_VERSION_MINOR}.${EmSMp_PROJECT_VERSION_PATCH}.${EmSMp_PROJECT_VERSION_REVISION}")

#     message(STATUS "${PROJECT_NAME} version is ${EmSMp_PROJECT_VERSION}")
#     message(STATUS "${PROJECT_NAME} full version is ${EmSMp_PROJECT_VERSION_FULL}")
# else()
#     message(WARNING "Can't generate project version/revision")
# endif()
# end generation project version

option(BUILD_DOCUMENTATION "Create and install documentation" OFF)

find_package(Boost COMPONENTS thread system regex REQUIRED)
add_subdirectory(protocol)
add_subdirectory(smlog)
add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)

option(BUILD_LIBRARY_CSERVER "Build CServer library" OFF)
if(BUILD_LIBRARY_CSERVER)
    add_subdirectory(cserver)
endif()

option(BUILD_EXAMPLES "Build examples" OFF)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
