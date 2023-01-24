cmake_minimum_required(VERSION 2.6)
project(voxel_carving)


if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 OLD)
	cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)


set (CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)
find_package(VTK REQUIRED)
include_directories(${OPENCV_INCLUDE_DIRS})
include(${VTK_USE_FILE})
include_directories(${VTK_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(${OpenCV_LIBRARY_DIRS})
link_directories(${VTK_LIBRARY_DIRS})
set(voxel_carving_src
    src/main.cpp
   )
add_executable(voxel_carving ${voxel_carving_src})
target_link_libraries(voxel_carving
    ${OpenCV_LIBRARIES}
    ${VTK_LIBRARIES})

target_compile_options(voxel_carving
    PRIVATE -O3 -std=c++17
    )