cmake_minimum_required (VERSION 3.6)
project(BingsuBot)

add_executable(BingsuBot main.cpp)

add_subdirectory(deps/sleepy-discord)
add_subdirectory(deps/cpp-dotenv)
target_link_libraries(BingsuBot cpp_dotenv)
target_link_libraries(BingsuBot sleepy-discord)