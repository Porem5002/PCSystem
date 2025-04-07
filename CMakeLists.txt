cmake_minimum_required(VERSION 3.14)

project(pcsystem)

add_compile_definitions("UNICODE" "_UNICODE")

add_library(pcsystem STATIC "pcsystem/pcsystem.hpp" "pcsystem/pcsystem.cpp")

add_executable(produtor "produtor.cpp")
target_link_libraries(produtor PUBLIC pcsystem)

add_executable(consumidor "consumidor.cpp")
target_link_libraries(consumidor PUBLIC pcsystem)