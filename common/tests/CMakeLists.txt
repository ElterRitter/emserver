cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include(CTest)
find_package(GTest REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

enable_testing()

set(TESTING_NAME "${PROJECT_NAME}_test")

set(${PROJECT_NAME}_tests_hdr test_protocol.h test_commonprocessor.h test_bufferprocessor.h)
set(${PROJECT_NAME}_tests_src test_protocol.cpp test_commonprocessor.cpp test_bufferprocessor.cpp)

add_executable(${TESTING_NAME} ${${PROJECT_NAME}_tests_hdr} ${${PROJECT_NAME}_tests_src}
     )
set_target_properties(${TESTING_NAME} PROPERTIES CXX_STANDARD 17
                                                 CXX_STANDARD_REQUIRED ON)

gtest_discover_tests(${TESTING_NAME})

target_link_libraries(${TESTING_NAME} PRIVATE TransportCommon GTest::GTest GTest::Main Boost::headers)

source_group("Headers" FILES ${${PROJECT_NAME}_tests_hdr})
source_group("Sources" FILES ${${PROJECT_NAME}_tests_src})

add_test(NAME ${TESTING_NAME}_tester COMMAND "${PROJECT_BINARY_DIR}/tests/${TESTING_NAME}")

set(RUN_TEST_TARGET_NAME "testing_${PROJECT_NAME}")
add_custom_target(${RUN_TEST_TARGET_NAME} ${CMAKE_CTEST_COMMAND})
set_target_properties(${RUN_TEST_TARGET_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_dependencies(${RUN_TEST_TARGET_NAME} ${TESTING_NAME})
