set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE SOURCES *.hpp *.cpp)
file(GLOB TEST_SOURCES ../unit_tests/*.*pp)

set(SOURCES ${SOURCES})
set(TEST_SOURCES ${TEST_SOURCES})

# Executable for running unit tests
add_executable(${BINARY}_test ${SOURCES} ${TEST_SOURCES})
# Library for linking to other projects
add_library(${BINARY}_lib STATIC ${SOURCES})

target_include_directories(${BINARY}_test PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

target_include_directories(${BINARY}_lib PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           ) 

target_link_libraries(${BINARY}_test gtest_main)
target_link_libraries(${BINARY}_test gmock_main)
