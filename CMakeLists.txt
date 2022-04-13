cmake_minimum_required(VERSION 3.21)
project(
    macos-sendfile-bug
    VERSION 1.0.0
    DESCRIPTION "Sample project for sendfile bug on MacOS"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)

add_executable(server server.c)
