cmake_minimum_required(VERSION 3.20)
project(emacs-cwd-by-pid)

add_library(emacs-get-cwd-by-pid SHARED
  src/get_process_cwd.cpp
  src/main.c)

target_compile_definitions(emacs-get-cwd-by-pid
  PUBLIC -DUNICODE=1
  PUBLIC -D_UNICODE=1
  PUBLIC -DNOMINMAX=1)
set_property(TARGET emacs-get-cwd-by-pid PROPERTY CXX_STANDARD 17)
