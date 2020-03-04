cmake_minimum_required(VERSION 3.00)

project(Odbc)

include_directories("./include")
link_directories("C:/Users/zhu/Desktop/Odbc/")

# aux_source_directory("./src" DolphinDB)


add_compile_options("-g" "-std=c++14" "-fPIC" "-DWINDOWS" "-Wall" "-D_WIN32_WINNT=0x0600" "-DWINVER=0x0600")

add_library(PluginOdbc SHARED "./src/DolphinDBODBC.cpp " "./src/nanodbc.cpp")
target_link_libraries(PluginOdbc libDolphinDB.dll odbc32 odbccp32)
