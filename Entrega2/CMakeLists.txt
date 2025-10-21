# CMakeLists.txt - Configuración de CMake para el proyecto tcp_chat_simple
# Genera dos ejecutables: server y client

cmake_minimum_required(VERSION 3.16)
project(tcp_chat_simple)

# Usar C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Flags de compilación
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")

# Ejecutable del servidor
add_executable(server server.cpp)

# Ejecutable del cliente
add_executable(client client.cpp)

# Mensajes informativos
message(STATUS "Configuración completada:")
message(STATUS "  - Ejecutable servidor: server")
message(STATUS "  - Ejecutable cliente: client")
message(STATUS "  - Estándar C++: ${CMAKE_CXX_STANDARD}")
