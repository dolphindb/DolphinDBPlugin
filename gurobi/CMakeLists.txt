cmake_minimum_required(VERSION 3.22)
set(PluginName PluginGurobi)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_include_directories(PluginGurobi PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/contrib/include)
target_link_directories(PluginGurobi PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/contrib/lib)
target_link_options(PluginGurobi PRIVATE "-Wl,-rpath,$ORIGIN,-disable-new-dtags")
target_link_libraries(PluginGurobi libgurobi_c++.a gurobi100)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/contrib/lib DESTINATION ${INSTALL_DIR})
