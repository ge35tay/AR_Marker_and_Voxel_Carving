cmake_minimum_required(VERSION 2.4)
project(generate_aruco_board)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 OLD)
	cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)

set (CMAKE_CXX_STANDARD 17)
#set(aruco_DIR /Users/hannahbell/AR_Marker_and_Voxel_Carving/ArUco/aruco-3.1.12/cmake)
find_package(OpenCV REQUIRED)
find_package(aruco REQUIRED)
include_directories(${OPENCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)


set(generate_aruco_src
    src/generate_aruco.cpp
)
add_executable(generate_aruco_board ${generate_aruco_src})
#target_link_libraries(generate_aruco_board ${OpenCV_LIBS} aruco::aruco)
target_link_libraries(generate_aruco_board ${OpenCV_LIBS} ${aruco_LIBS})

target_compile_options(generate_aruco_board
    PRIVATE -O3 -std=c++17
    )