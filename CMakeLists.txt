cmake_minimum_required(VERSION 3.20)
project(emacscwd)

add_executable(emacscwd WIN32
  src/get_process_cwd.cpp
  src/test.c)

target_compile_definitions(emacscwd
  PUBLIC -DUNICODE=1
  PUBLIC -D_UNICODE=1
  PUBLIC -DNOMINMAX=1)
target_link_options(emacscwd
  PRIVATE /machine:x64 /subsystem:console /incremental:no)
target_link_libraries(emacscwd
  PRIVATE ntdll)
set_property(TARGET emacscwd PROPERTY CXX_STANDARD 17)
