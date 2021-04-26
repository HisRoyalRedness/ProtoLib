set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

set(SOURCES ${SOURCES})

add_executable(${BINARY}_run ${SOURCES})

add_library(${BINARY}_lib STATIC ${SOURCES})

#configure_file(BuildVer.h.in BuildVer.h)

target_include_directories(${BINARY}_run PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

target_include_directories(${BINARY}_lib PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )