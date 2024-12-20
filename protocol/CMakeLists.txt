cmake_minimum_required(VERSION 3.14)

project(Protocol LANGUAGES CXX VERSION 0.0.1)

set(USE_CONFIG)
if(WIN32)
    set(USE_CONFIG "CONFIG")
endif()

find_package(Protobuf ${USE_CONFIG} REQUIRED)

set(PROTO_HDRS)
set(PROTO_SRCS)
set(PROTO_FILES "${PROJECT_SOURCE_DIR}/proto/capabilities.proto"
                "${PROJECT_SOURCE_DIR}/proto/common.proto"
                "${PROJECT_SOURCE_DIR}/proto/emserver.proto"
                "${PROJECT_SOURCE_DIR}/proto/capabilities.proto"
                "${PROJECT_SOURCE_DIR}/proto/sensors.proto")

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/proto")
if(TARGET protobuf::protoc)
    message(STATUS "Generate protocol buffers classes")
    get_target_property(PROTO_COMPILER protobuf::protoc "LOCATION_${CMAKE_BUILD_TYPE}")
    foreach(ProtoFile IN LISTS PROTO_FILES)
        get_filename_component(PROTO_NAME ${ProtoFile} NAME_WE)

        set(ProtoHdr ${PROJECT_BINARY_DIR}/proto/${PROTO_NAME}.pb.h)
        set(ProtoSrc ${PROJECT_BINARY_DIR}/proto/${PROTO_NAME}.pb.cc)
        add_custom_command(OUTPUT ${ProtoSrc} ${ProtoHdr}
                           COMMAND ${PROTO_COMPILER} "--proto_path=${PROJECT_SOURCE_DIR}/proto" "--cpp_out=${PROJECT_BINARY_DIR}/proto" ${ProtoFile}
                           DEPENDS ${ProtoFile}
                           COMMENT "Generate C++ protocol buffers classes for ${ProtoFile}"
                           VERBATIM)
        list(APPEND PROTO_HDRS ${ProtoHdr})
        list(APPEND PROTO_SRCS ${ProtoSrc})
    endforeach()
    set_source_files_properties(${PROTO_HDRS} ${PROTO_SRCS} PROPERTIES GENERATED TRUE
                                                                       SKIP_AUTOMOC ON
                                                                       SKIP_AUTOUIC ON
                                                                       SKIP_AUTORCC ON)
else()
    message(FATAL_ERROR "Can't find protocol buffers compiler")
endif()
add_library(${PROJECT_NAME} STATIC ${PROTO_SRCS} ${PROTO_HDRS} ${PROTO_FILES})
target_link_libraries(${PROJECT_NAME} protobuf::libprotobuf)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR};${PROJECT_BINARY_DIR}/proto")

message(VERBOSE "Generated hdrs = ${PROTO_HDRS}")
message(VERBOSE "Generated srcs = ${PROTO_SRCS}")

source_group("Generated" FILES ${PROTO_SRCS})
source_group("Proto" FILES ${PROTO_FILES})
