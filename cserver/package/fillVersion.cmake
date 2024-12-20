cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

if(WIN32)
    message(STATUS "CSERVER_VERSION = ${CSERVER_VERSION}; CONFIG_FILE_DIR = ${CONFIG_FILE_DIR}")
    configure_file("${CONFIG_FILE_DIR}/cserver.nuspec.in" ${CMAKE_CURRENT_BINARY_DIR}/cserver.nuspec @ONLY)
elseif(UNIX)
    message(STATUS "Try to packaging CServer library on linux")
    configure_file("${CONFIG_FILE_DIR}/cserver.control.in" ${CMAKE_CURRENT_BINARY_DIR}/deb_package/DEBIAN/control @ONLY)
endif()
