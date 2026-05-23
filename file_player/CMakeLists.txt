cmake_minimum_required(VERSION 3.8)
project(file_player)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(rosgraph_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(image_transport REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PCL REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(pcl_msgs REQUIRED)
find_package(irp_sen_msgs REQUIRED)
find_package(livox_ros_driver REQUIRED)
find_package(camera_info_manager REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets Gui OpenGL)
find_package(OpenGL REQUIRED)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FILE_PLAYER_SRCS
  ${SRC_DIR}/main.cpp
  ${SRC_DIR}/mainwindow.cpp
  ${SRC_DIR}/ROSThread.cpp
)
set(FILE_PLAYER_HDRS
  ${SRC_DIR}/mainwindow.h
  ${SRC_DIR}/ROSThread.h
)
set(FILE_PLAYER_UI ${SRC_DIR}/mainwindow.ui)

qt5_wrap_ui(FILE_PLAYER_UI_MOC ${FILE_PLAYER_UI})
qt5_add_resources(SHADER_RSC_ADDED ${SRC_DIR}/resources.qrc)

add_executable(file_player
  ${FILE_PLAYER_SRCS}
  ${FILE_PLAYER_HDRS}
  ${FILE_PLAYER_UI_MOC}
  ${SHADER_RSC_ADDED}
)

target_include_directories(file_player PRIVATE
  ${SRC_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS}
  ${PCL_INCLUDE_DIRS}
)

ament_target_dependencies(file_player
  rclcpp
  std_msgs
  geometry_msgs
  sensor_msgs
  nav_msgs
  rosgraph_msgs
  std_srvs
  image_transport
  cv_bridge
  pcl_conversions
  pcl_msgs
  irp_sen_msgs
  livox_ros_driver
  camera_info_manager
)

target_link_libraries(file_player
  Qt5::Widgets
  Qt5::Gui
  Qt5::OpenGL
  ${OpenCV_LIBRARIES}
  ${PCL_LIBRARIES}
  OpenGL::GL
)

install(TARGETS file_player
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

ament_package()
