cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Debug)

project(FileManagerProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)	
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(FileManager STATIC IMPORTED) 
set_target_properties(FileManager PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libFileManager.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/"
)

# Ejecutable del servidor
add_executable(server utils.h utils.cpp server.cpp clientManager.h clientManager.cpp filemanager.h)
target_link_libraries(server pthread FileManager stdc++fs)

# Ejecutable del cliente
add_executable(client utils.h utils.cpp client.cpp clientManager.h distributedFileManager.cpp filemanager.h)
target_link_libraries(client pthread stdc++fs)

# Ejecutable del fileManager original (programa de prueba)
add_executable(fileManager main_fm.cpp filemanager.h)
target_link_libraries(fileManager pthread FileManager stdc++fs)
