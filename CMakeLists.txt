cmake_minimum_required(VERSION 3.15)
project(ssm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find CURL package
find_package(CURL REQUIRED)
find_package(libzip REQUIRED)

# Create the executable first
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
add_executable(ssm ${SRC_FILES})

# Then link libraries to it
target_link_libraries(ssm PRIVATE CURL::libcurl)
target_link_libraries(ssm PRIVATE libzip::zip)

# Include directories (modern CMake prefers target_include_directories)
target_include_directories(ssm PRIVATE 
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)