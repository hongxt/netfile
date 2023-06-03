cmake_minimum_required(VERSION 3.25)
project(FileR)

set(CMAKE_CXX_STANDARD 14)




add_executable(myFileRR  myFileRR.cpp)
target_link_libraries(myFileRR ws2_32.lib)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

add_executable(myFileSS myFileSS.cpp)
target_link_libraries(myFileSS ws2_32.lib)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")