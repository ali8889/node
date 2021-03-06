cmake_minimum_required(VERSION 3.10)

project(net)

add_library(net
  include/net/neighbourhood.hpp
  include/net/network.hpp
  include/net/packet.hpp
  include/net/pacmans.hpp
  include/net/transport.hpp
  include/net/logger.hpp
  include/net/packetvalidator.hpp
  src/neighbourhood.cpp
  src/network.cpp
  src/packet.cpp
  src/pacmans.cpp
  src/transport.cpp
  src/packetvalidator.cpp
)

add_dependencies(${PROJECT_NAME} csconnector)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                      ${CMAKE_CURRENT_SOURCE_DIR}/include/net
                                      ${CMAKE_CURRENT_SOURCE_DIR}/../api/include
                                      ${CMAKE_CURRENT_SOURCE_DIR}/../client/include
                                      ${CMAKE_CURRENT_SOURCE_DIR}/../csnode/include
                                      ${CMAKE_CURRENT_SOURCE_DIR}/../csdb/include
                                      ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/thrift/lib/cpp/src
                                      ${CMAKE_CURRENT_SOURCE_DIR}/../csdb
                                      ${CMAKE_BINARY_DIR}/api/api_gen
                                      ${CMAKE_BINARY_DIR}/api/executor_gen
                                      ${CMAKE_BINARY_DIR}/third-party/thrift
                                      ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/lz4
                                      ${CMAKE_CURRENT_SOURCE_DIR}/../solver2/include
                                      ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)

suppress_boost_cmake_warnings()
configure_msvc_flags()

target_link_libraries (net csnode lib cscrypto)

set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
if(NOT APPLE)
  set (Boost_USE_STATIC_RUNTIME ON)
endif()

find_package (Boost REQUIRED COMPONENTS system filesystem)
target_link_libraries (net Boost::system Boost::filesystem Boost::disable_autolinking)
