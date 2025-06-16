set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.16)
project(partitioner VERSION 0.1.0 LANGUAGES C CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()
file(GLOB files src/*.cpp)
qt_add_executable(partitioner ${files})
target_include_directories(partitioner PRIVATE include)

include(CTest)
enable_testing()

target_link_libraries(partitioner PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

