add_executable(echo echo.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src)
target_link_libraries(echo n2t)
