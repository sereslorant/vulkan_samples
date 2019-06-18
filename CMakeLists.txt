cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 17)

include(ExternalProject)

ExternalProject_Add(libsdl2
                    URL https://libsdl.org/release/SDL2-2.0.7.tar.gz
                    INSTALL_DIR ${PROJECT_BINARY_DIR}
                    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                    )


ExternalProject_Add(glslang
                    URL https://github.com/KhronosGroup/glslang/archive/master.tar.gz
                    INSTALL_DIR ${PROJECT_BINARY_DIR}
                    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                    )

find_package(Threads)

include_directories(${PROJECT_BINARY_DIR}/include ./)
link_directories(${PROJECT_BINARY_DIR}/lib)

add_subdirectory(lvkUtils)

add_subdirectory(CliTest)
add_subdirectory(lvkSampleApps)
