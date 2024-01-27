cmake_minimum_required(VERSION 3.27)
project(remote-unwinder)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/abseil-cpp)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/dylib.hpp
    src/unwinder.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE external/unwinding/libunwindstack/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE log absl::strings)
target_compile_definitions(${PROJECT_NAME} PRIVATE NO_DEBUG)
