cmake_minimum_required(VERSION 3.15)
project(ssm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(spdlog REQUIRED)
find_package(CURL REQUIRED)
find_package(libzip REQUIRED)
find_package(pugixml REQUIRED)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(ssm ${SRC_FILES})

target_link_libraries(ssm PRIVATE spdlog::spdlog)
target_link_libraries(ssm PRIVATE CURL::libcurl)
target_link_libraries(ssm PRIVATE libzip::zip)
target_link_libraries(ssm PRIVATE pugixml::pugixml)

target_include_directories(ssm PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(UNIX AND CMAKE_EXPORT_COMPILE_COMMANDS)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
endif()