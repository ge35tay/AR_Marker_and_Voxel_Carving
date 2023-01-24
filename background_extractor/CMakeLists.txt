cmake_minimum_required(VERSION 2.4)
project(background_extractor)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 OLD)
	cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)

set (CMAKE_CXX_STANDARD 17)
find_package(OpenCV REQUIRED)

include_directories(${OPENCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(${OpenCV_LIBRARY_DIRS})

set(background_extractor_src
    src/main.cpp
   )
add_executable(background_extractor ${background_extractor_src})
target_link_libraries(background_extractor
    ${OpenCV_LIBRARIES}
    )

target_compile_options(background_extractor
    PRIVATE -O3 -std=c++17
    )

