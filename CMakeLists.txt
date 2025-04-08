cmake_minimum_required(VERSION 3.14)

project(pcsystem)

add_compile_definitions("UNICODE" "_UNICODE")

set(SHARED_SOURCES "src/pcsystem/pcsystem.hpp" "src/shared.hpp" "src/shared.cpp")

add_executable(produtor ${SHARED_SOURCES} "src/produtor.cpp")
add_executable(consumidor ${SHARED_SOURCES} "src/consumidor.cpp")
