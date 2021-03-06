cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE Release)
project(OpenKAI)

add_definitions("-O3 -w -Wall -c -fmessage-length=0 -MMD -MP -std=c++11")

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

OPTION(USE_CUDA "Use NVIDIA CUDA" OFF)
OPTION(USE_OPENCV_CONTRIB "Use OpenCV contribution" OFF)
OPTION(USE_DARKNET "Use Darknet" OFF)
OPTION(USE_REALSENSE "Use Intel RealSense camera" OFF)
OPTION(USE_OPENTRACKER "Use OpenTracker" OFF)
OPTION(USE_PYLON "Use Basler camera with Pylon" OFF)
OPTION(USE_TENSORRT "Use NVIDIA TensorRT" OFF)
OPTION(USE_CAFFE "Use Caffe" OFF)
OPTION(USE_ORB_SLAM2 "Use Orb_Slam2" OFF)
OPTION(USE_OCR "Use Tesseract" OFF)
OPTION(USE_OPENALPR "Use OpenALPR" OFF)
OPTION(USE_DYNAMIXEL "Use Dynamixel" OFF)
OPTION(USE_LIVOX "Use Livox" OFF)
OPTION(USE_MYNTEYE "Use Mynteye" OFF)

find_package(Eigen3 3.1.0 REQUIRED)
# find_package(OpenCV 4 REQUIRED)

set(CUDA_include "/usr/local/cuda/include" CACHE STRING "")
set(OpenKAI_libDir /usr/lib /usr/local/lib)
set(OpenKAI_include /usr/local/include /usr/include ${EIGEN3_INCLUDE_DIR})
set(OpenKAI_lib pthread boost_system glog uuid ${EIGEN3_LIBS} gsl gslcblas ncurses)

set(OpenCV_include "/usr/local/include/opencv4" CACHE STRING "")
set(OpenCV_lib "/usr/local/lib" CACHE STRING "")
set(OpenKAI_libDir ${OpenKAI_libDir} ${OpenCV_lib})
set(OpenKAI_include ${OpenKAI_include} ${OpenCV_include})
set(OpenKAI_lib ${OpenKAI_lib} opencv_core opencv_imgcodecs opencv_highgui opencv_imgproc opencv_videoio opencv_calib3d opencv_objdetect opencv_features2d opencv_dnn)

if(USE_CUDA)
  add_definitions(-DUSE_CUDA)
  set(OpenKAI_include ${OpenKAI_include} ${CUDA_include})
  set(CUDA_ARCH "62" CACHE STRING "")
  set(OpenKAI_lib ${OpenKAI_lib} opencv_cudawarping opencv_cudaobjdetect opencv_cudaarithm opencv_cudabgsegm opencv_cudaimgproc opencv_cudaoptflow opencv_cudastereo)
endif()

if(USE_OPENCV_CONTRIB)
  add_definitions(-DUSE_OPENCV_CONTRIB)
  set(OpenKAI_lib ${OpenKAI_lib} opencv_xobjdetect opencv_tracking opencv_aruco)
endif()

if(USE_DARKNET)
  add_definitions(-DUSE_DARKNET)  
  set(Darknet_root "/darknet" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} ${Darknet_root})
  set(OpenKAI_include ${OpenKAI_include} "${Darknet_root}/api")
  set(OpenKAI_lib ${OpenKAI_lib} darknet)
endif()

if(USE_OPENTRACKER)
  add_definitions(-DUSE_OPENTRACKER)  
  set(OpenTracker_root "/OpenTracker" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} "${OpenTracker_root}/eco")
  set(OpenKAI_include ${OpenKAI_include} "${OpenTracker_root}/eco")
  set(OpenKAI_lib ${OpenKAI_lib} ecotracker)
endif()

if(USE_MYNTEYE)
  add_definitions(-DUSE_MYNTEYE)
  set(OpenKAI_lib ${OpenKAI_lib} mynteye_depth)
endif()

if(USE_REALSENSE)
  add_definitions(-DUSE_REALSENSE)
  set(librealsense_root "/librealsense" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} "${librealsense_root}/build")
  set(OpenKAI_include ${OpenKAI_include} "${librealsense_root}/include")
  set(OpenKAI_lib ${OpenKAI_lib} realsense2)
endif()

if(USE_TENSORRT)
  add_definitions(-DUSE_TENSORRT)  
  set(TensorRT_build /jetson-inference-batch/build/${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} "${TensorRT_build}/lib")
  set(OpenKAI_include ${OpenKAI_include} "${TensorRT_build}/include")
  set(OpenKAI_lib ${OpenKAI_lib} jetson-inference)
endif()

if(USE_CAFFE)
  add_definitions(-DUSE_CAFFE)
  set(Caffe_distribute "/caffe/distribute" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} "${Caffe_distribute}/lib")
  set(OpenKAI_include ${OpenKAI_include} "${Caffe_distribute}/include")
  set(OpenKAI_lib ${OpenKAI_lib} caffe)
endif()

if(USE_PYLON)
  add_definitions(-DUSE_PYLON)
  set(Pylon_root "/opt/pylon5" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} "${Pylon_root}/lib64")
  set(OpenKAI_include ${OpenKAI_include} "${Pylon_root}/include")
  set(OpenKAI_lib ${OpenKAI_lib} Log_gcc_v3_0_Basler_pylon_v5_0 MathParser_gcc_v3_0_Basler_pylon_v5_0 XmlParser_gcc_v3_0_Basler_pylon_v5_0 NodeMapData_gcc_v3_0_Basler_pylon_v5_0 pylonc pylonbase pylonutility GenApi_gcc_v3_0_Basler_pylon_v5_0 GCBase_gcc_v3_0_Basler_pylon_v5_0)
endif()

if(USE_ORB_SLAM2)
  find_package(Pangolin REQUIRED)
  add_definitions(-DUSE_ORB_SLAM2)
  set(ORB_SLAM2_root "/orb_slam2_gpu" CACHE STRING "")
  set(OpenKAI_include ${OpenKAI_include} "${ORB_SLAM2_root} ${ORB_SLAM2_root}/include ${Pangolin_INCLUDE_DIRS}")
  set(OpenKAI_libDir ${OpenKAI_libDir} "${ORB_SLAM2_root}/build/lib")
  set(OpenKAI_lib ${OpenKAI_lib} ${Pangolin_LIBRARIES} ORB_SLAM2)
endif()

if(USE_OCR)
  add_definitions(-DUSE_OCR)
  set(OpenKAI_lib ${OpenKAI_lib} tesseract)
endif()

if(USE_OPENALPR)
  add_definitions(-DUSE_OPENALPR)
  set(OpenKAI_lib ${OpenKAI_lib} openalpr)
endif()

if(USE_DYNAMIXEL)
  add_definitions(-DUSE_DYNAMIXEL)
  set(OpenKAI_lib ${OpenKAI_lib} dxl_x64_cpp)
endif()

if(USE_LIVOX)
  add_definitions(-DUSE_LIVOX)
  set(OpenKAI_lib ${OpenKAI_lib} livox_sdk_static apr-1)
endif()


# Setting specific lib here
link_directories(${OpenKAI_libDir})

if(USE_CUDA)
find_package(CUDA)
set(
	CUDA_NVCC_FLAGS
	${CUDA_NVCC_FLAGS}; 
    -O3 -gencode arch=compute_${CUDA_ARCH},code=sm_${CUDA_ARCH}
)
endif()

# File
file(GLOB OpenKAI_cpp src/*.cpp src/*/*.cpp src/*/*/*.cpp src/*/*/*/*.cpp src/*/*/*/*/*.cpp src/*/*/*/*/*/*.cpp src/*/*/*/*/*/*/*.cpp src/*.c src/*/*.c src/*/*/*.c src/*/*/*/*.c src/*/*/*/*/*.c src/*/*/*/*/*/*.c src/*/*/*/*/*/*/*.c src/*.cc src/*/*.cc src/*/*/*.cc src/*/*/*/*.cc src/*/*/*/*/*.cc src/*/*/*/*/*/*.cc src/*/*/*/*/*/*/*.cc)

if(USE_CUDA)
cuda_add_executable(OpenKAI ${OpenKAI_cpp})
else()
add_executable(OpenKAI ${OpenKAI_cpp})
endif()

# C/C++ settings
include_directories(${OpenKAI_include})
target_link_libraries(OpenKAI ${OpenKAI_lib})
