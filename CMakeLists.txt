cmake_minimum_required(VERSION 4.2)
project(Honesto)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Adicionamos o resource.rc na lista de arquivos do executável
add_executable(Honesto main.cpp resource.rc)

if(MINGW)
    target_link_options(Honesto PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
endif()

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()