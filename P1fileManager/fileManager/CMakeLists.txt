#Rodrigo Fernández
#05/11/2205

cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Debug)

project(server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)	
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(FileManager STATIC IMPORTED) 
set_target_properties(FileManager PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libFileManager.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/"
)

project(broker LANGUAGES CXX)
add_executable(broker utils.h utils.cpp broker.cpp brokerManager.cpp brokerManager.h)
target_link_libraries(broker pthread stdc++fs)

project(server LANGUAGES CXX)
add_executable(server utils.h utils.cpp server.cpp clientManager.cpp clientManager.h brokerManager.h fileManager.h)
target_link_libraries(server pthread  FileManager stdc++fs)


project(client LANGUAGES CXX)
add_executable(client utils.cpp main_fm.cpp distributedFileManager.cpp clientManager.h brokerManager.h fileManager.h)
target_link_libraries(client pthread stdc++fs)


project (fileManager)
add_executable (fileManager main_fm.cpp fileManager.h)
target_link_libraries(fileManager pthread FileManager stdc++fs)






