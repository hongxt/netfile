cmake_minimum_required(VERSION 3.15)
project(FileRS)

set(CMAKE_CXX_STANDARD 14)



add_executable(myFileSS myclient.cpp)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

add_executable(myFileRR myserver.cpp)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

