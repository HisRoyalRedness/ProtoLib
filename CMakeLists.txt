# CMakeList.txt : CMake project for ProtoLib, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.18)

project (ProtoLib VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED true)

#file(MAKE_DIRECTORY unit_test)
#file(MAKE_DIRECTORY lib/googletest)

include_directories(src)

add_subdirectory(src)


# configure a header file to pass some of the CMake settings
# to the source code
configure_file(BuildVer.h.in BuildVer.h)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# TODO: Add tests and install targets if needed.

#target_include_directories(ProtoLib PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#                           )

