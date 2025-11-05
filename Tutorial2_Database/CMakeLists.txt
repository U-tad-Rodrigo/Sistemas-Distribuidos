 
cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(userInterfaceLocal LANGUAGES CXX)
add_executable(userInterfaceLocal userInterfaceLocal.cpp database.cpp database.h)
target_link_libraries(userInterfaceLocal pthread)

project(userInterfaceClient LANGUAGES CXX)
add_executable(userInterfaceClient userInterfaceLocal.cpp distributedDatabase.cpp database.h utils.h utils.cpp)
target_link_libraries(userInterfaceClient pthread)

project(server LANGUAGES CXX)
add_executable(server server.cpp clientManager.cpp database.cpp database.h utils.h utils.cpp)
target_link_libraries(server pthread)

