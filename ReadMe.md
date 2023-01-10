## 1. Weekly Progress

> https://docs.google.com/document/d/1KwS3K1Sy-X0S0QYXjDn_9hW1t9BkSR6E7a4P1YrBrGE/edit



## 2. 

ArUco generation

> https://docs.opencv.org/4.x/d5/dae/tutorial_aruco_detection.html

youtube:

1. ArUco lib

> https://www.youtube.com/watch?v=aWmzhmfahh8

2. Pose esitmation

> https://www.youtube.com/watch?v=GEWoGDdjlSc





## 3. Install opencv
   1. Ubuntu 20.04
   > sudo apt install build-essential cmake git pkg-config libgtk-3-dev \
    libavcodec-dev libavformat-dev libswscale-dev libv4l-dev \
    libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev \
    gfortran openexr libatlas-base-dev python3-dev python3-numpy \
    libtbb2 libtbb-dev libdc1394-22-dev libopenexr-dev \
    libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev

> mkdir ~/opencv_build && cd ~/opencv_build
>
> git clone https://github.com/opencv/opencv.git
>
> git clone https://github.com/opencv/opencv_contrib.git

>```
>cd ~/opencv_build/opencv
>mkdir -p build && cd build
>```

> ```
> cmake -D CMAKE_BUILD_TYPE=RELEASE \    -D CMAKE_INSTALL_PREFIX=/usr/local \    -D INSTALL_C_EXAMPLES=ON \    -D INSTALL_PYTHON_EXAMPLES=ON \    -D OPENCV_GENERATE_PKGCONFIG=ON \    -D OPENCV_EXTRA_MODULES_PATH=~/opencv_build/opencv_contrib/modules \    -D BUILD_EXAMPLES=ON ..
> ```

> make -j8
>
> sudo make install



## 4. Milestone

1. week 1
   1. install all the libraries and make sure it can run
   2. 相机内参标定， pose tracking
2. week 2-3
   1. 看carving的论文
   2. 先针对比较规则的物体进行无颜色信息的carving
   3. 颜色渲染
3. week 4
   1. 复杂物体？




## 5. generate aruco board
```
cd generate_aruco_board 
mkdir build && cd build 
cmake .. 
make 
./generate_board --bb=1 -h=4 -w=4 -l=50 -s=200 -d=16 --si board.jpg
```

## 6. detect marker

```
cd detect_marker 
mkdir build && cd build 
cmake .. 
make 
./detect_marker -v=2 
```

## 7. camera calibration

```
cd camera_calibration 
mkdir build && cd build 
cmake .. 
make 
./camera_calibration -d=16 -dp=../detector_params.yml -h=4 -w=4 -l=50 -s=200 ./calibration_params.yml -v=2 -ci=2
```

## 8. Pose Estimation
```
cd pose_estimation
mkdir build && cd build
cmake ..
make
./pose_estimation -d=16 -h=4 -w=4 -l=50 -s=200 -v=2
```


## Camera RT
> https://stackoverflow.com/questions/73999642/camera-pose-estimation-accuracy-with-aruco-board





## 9 Background Extractor

bad performance need improvement

```
cd background_extractor
mkdir build && cd build
cmake ..
make
./background_extractor --video=1 -image=0 -algo=MOG2
```



## 10 Voxel carving

prerequisite

```
sudo apt-get install build-essential libgl1-mesa-dev
sudo apt-get install freeglut3-dev
sudo apt-get install libglew-dev libsdl2-dev libsdl2-image-dev libglm-dev libfreetype6-dev

sudo apt install ffmpeg libavcodec-dev libavformat-dev libavfilter-dev
```

Qt5

https://kenny7012.medium.com/%E5%A6%82%E4%BD%95%E5%9C%A8ubuntu-20-04%E5%AE%89%E8%A3%9Dqt5-12-0-c3d2a9a4bd3f

vtk

VTK source

> https://www.vtk.org/files/release/9.2/VTK-9.2.0.rc2.tar.gz

VTK data file

> https://www.vtk.org/files/release/9.2/VTK-9.2.0.rc2.tar.gz

unzip both file

```
gunzip VTK-9.2.0.rc2.tar.gz
gunzip VTKData-9.2.0.rc2.tar
```



then build VTK

```
cd VTK-9.2.0.rc2
mkdir build && cd build

cmake -DVTK_GROUP_ENABLE_Qt=YES -DVTK_MODULE_ENABLE_VTK_GUISupportQt=YES -DModule_vtkGUISupportQt=ON -DModule_vtkGUISupportQtOpenGL=ON -DModule_vtkGUISupportQtSQL=ON -DModule_vtkRenderingQt=ON -DModule_vtkViewsQt=ON -DVTK_RENDERING_BACKEND=OpenGL -DVTK_QT_VERSION=5 -DQt5_DIR:PATH=/home/yinghanhuang/Qt5.12.11/5.12.11/gcc_64/lib/cmake/Qt5
 -DQT_QMAKE_EXECUTABLE=/home/yinghanhuang/Qt5.12.11/5.12.11/gcc_64/bin/qmake -DCMAKE_PREFIX_PATH=/home/yinghanhuang/Qt5.12.11/5.12.11/gcc_64/lib/cmake -DBUILD_EXAMPLES=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release ..
 
 
 make -j8
 sudo make install
```



to complie

```
cd voxel_carving
mkdir build && cd build
cmake ..
make
./voxel_carving
```

