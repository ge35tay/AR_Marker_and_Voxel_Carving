cmake_minimum_required(VERSION 2.4)
project(combine_ar)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 OLD)
	cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)

set (CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)
include_directories(${OPENCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

aux_source_directory(src DIR_SRC)
set(ar_src ${DIR_SRC})
add_executable(ar ${ar_src})
target_link_libraries(ar
    ${OpenCV_LIBRARIES}
    )

target_compile_options(ar
    PRIVATE -O3 -std=c++17
    )