cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)

add_executable(server main.cpp)
find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)

target_link_libraries(server pthread curl ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})