cmake_minimum_required(VERSION 3.10)
project(NativeCodeEditor)

set(CMAKE_CXX_STANDARD 17)

add_executable(NativeCodeEditor 
    main.cpp 
    editor.cpp 
    editor.h
)
