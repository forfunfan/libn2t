add_executable(tun2socks-ng tun2socks-ng.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src)
target_link_libraries(tun2socks-ng n2t)
