# HD convert ROS1 -> ROS2 Jazzy (Ubuntu 24.04) cho MA_LIO

## 1) Mục tiêu
- Chuyển package `MA_LIO` từ ROS1 (catkin) sang ROS2 Jazzy (ament_cmake).
- Chuẩn hóa chuẩn biên dịch C/C++17.
- Hướng dẫn build và chạy launch trên ROS2.
- Hướng dẫn phát dataset `City02` bằng ROS2 (publisher topic từ dữ liệu file).
- Ghi nhận **các API ROS1 còn tồn đọng** để tiếp tục migration.

---

## 2) Hiện trạng code (audit nhanh)
Các điểm ROS1 được phát hiện:
- Build system: `catkin`, `message_generation`, `roscpp`, `tf`.
- Source dùng trực tiếp ROS1 API: `ros::NodeHandle`, `ros::Publisher`, `ros::Subscriber`, `ros::spinOnce`, `ros::Rate`, `ROS_WARN/ROS_INFO`, `ros::Time`.
- Message sync dùng `message_filters` kiểu ROS1.
- TF dùng `tf::TransformBroadcaster` (ROS1 tf cũ).
- LiDAR msg dùng `livox_ros_driver/CustomMsg` (ROS1 package).

---

## 3) Những thay đổi đã thực hiện trong repo (giai đoạn 1)

### 3.1 Build system ROS2 cho package `ma_lio`
Đã chuyển `MA_LIO/CMakeLists.txt` sang cấu trúc `ament_cmake`:
- `cmake_minimum_required(VERSION 3.8)`.
- `CMAKE_C_STANDARD 17`, `CMAKE_CXX_STANDARD 17`.
- Dùng `find_package(... REQUIRED)` cho ROS2 deps (`rclcpp`, `sensor_msgs`, `nav_msgs`, `geometry_msgs`, `tf2`, `tf2_ros`, `pcl_conversions`, `message_filters`, ...).
- Dùng `rosidl_generate_interfaces` để build msg `Pose6D.msg`.
- Export dependency bằng `ament_export_dependencies`.
- Install binary + config/launch/rviz.

> Lưu ý: giai đoạn này mới là khung build ROS2. Source chính vẫn cần migrate API ROS1 -> ROS2 để build thành công hoàn toàn.

### 3.2 Manifest ROS2
Đã chuyển `MA_LIO/package.xml` sang format ROS2:
- `buildtool_depend` = `ament_cmake`.
- thêm `rosidl_default_generators`, `rosidl_default_runtime`.
- dùng dependency ROS2 tương ứng.

### 3.3 Launch ROS2
Đã thêm launch ROS2 Python:
- `MA_LIO/launch/mapping_city.launch.py`
- Dùng `launch_ros.actions.Node` chạy executable `malio_mapping`.
- Nạp file config `config/City.yaml`.

---

## 4) API ROS1 còn tồn đọng (cần xử lý tiếp)

### 4.1 Trong `MA_LIO/src/laserMapping.cpp`
- Header ROS1:
  - `#include <ros/ros.h>`
  - `#include <tf/transform_datatypes.h>`
  - `#include <tf/transform_broadcaster.h>`
- ROS1 runtime:
  - `ros::init`, `ros::NodeHandle`, `ros::Subscriber`, `ros::Publisher`, `ros::Rate`, `ros::ok`, `ros::spinOnce`.
- Time API:
  - `ros::Time::now()`, `ros::Time().fromSec(...)`.
- Logging macro:
  - `ROS_WARN`, `ROS_INFO`.
- TF broadcaster:
  - `tf::TransformBroadcaster`, `tf::Transform`, `tf::Quaternion`.

### 4.2 Trong `MA_LIO/src/preprocess.h` và `MA_LIO/src/preprocess.cpp`
- `#include <ros/ros.h>`.
- `ros::Publisher` members.
- callback signatures dùng `ConstPtr` kiểu ROS1.
- `ros::Time` trong hàm publish.

### 4.3 Trong `MA_LIO/src/parameters.h/.cpp`
- `readParameters(ros::NodeHandle&)`.

### 4.4 Trong `MA_LIO/src/IMU_Processing.hpp`
- include ROS1 + logging macro ROS1.

### 4.5 Vấn đề driver/msg Livox
- Đang dùng `livox_ros_driver/CustomMsg` (ROS1).
- Với ROS2 Jazzy cần:
  - dùng bản ROS2 của Livox driver (nếu tương thích), hoặc
  - tạo bridge/adapter message package để map dữ liệu sang `sensor_msgs/msg/PointCloud2` + time fields tương ứng.

---

## 5) Mapping API ROS1 -> ROS2 (đề xuất triển khai)

- `ros::NodeHandle` -> class node kế thừa `rclcpp::Node`.
- `ros::Publisher<T>` -> `rclcpp::Publisher<T>::SharedPtr`.
- `ros::Subscriber` -> `rclcpp::Subscription<T>::SharedPtr`.
- `ros::Time` -> `rclcpp::Time`, `this->now()`.
- `ros::Rate` -> `rclcpp::WallRate`.
- `ros::spinOnce()` -> `rclcpp::spin_some(node)` hoặc executor.
- `ROS_INFO/WARN` -> `RCLCPP_INFO/WARN(this->get_logger(), ...)`.
- `tf` -> `tf2` + `tf2_ros::TransformBroadcaster` + `geometry_msgs::msg::TransformStamped`.
- callback `Msg::ConstPtr` -> `Msg::SharedPtr` (hoặc `ConstSharedPtr`).

---

## 6) Build ROS2 Jazzy trên Ubuntu 24.04 (khuyến nghị thực tế)

### 6.1 Cài dependency
```bash
sudo apt update
sudo apt install -y \
  ros-jazzy-desktop \
  ros-jazzy-pcl-conversions \
  ros-jazzy-pcl-msgs \
  ros-jazzy-tf2-ros \
  ros-jazzy-message-filters \
  ros-jazzy-rosidl-default-generators \
  libpcl-dev libeigen3-dev
```

### 6.2 Build workspace
```bash
source /opt/ros/jazzy/setup.bash
cd <ros2_ws>
colcon build --symlink-install --packages-select irp_sen_msgs ma_lio city02_player_py
source install/setup.bash
```

> Nếu build fail tại `ma_lio`, nguyên nhân thường do API ROS1 còn trong mã C++ (xem mục 4).

---

## 7) Chạy thuật toán + dataset City02 bằng ROS2

### 7.1 Chạy riêng từng launch
Terminal 1 (player City02):
```bash
source /opt/ros/jazzy/setup.bash
source <ros2_ws>/install/setup.bash
ros2 launch city02_player_py city02_player.launch.py \
  root:=/path/to/City02/sensor_data rate:=1.0
```

Terminal 2 (MA_LIO):
```bash
source /opt/ros/jazzy/setup.bash
source <ros2_ws>/install/setup.bash
ros2 launch ma_lio mapping_city.launch.py
```

### 7.2 Chạy launch gộp (mới thêm)
Đã thêm `MA_LIO/launch/city02_mapping.launch.py` để chạy đồng thời:
- node `city02_player_py/city02_player_node`
- node `ma_lio/malio_mapping`

Lệnh:
```bash
source /opt/ros/jazzy/setup.bash
source <ros2_ws>/install/setup.bash
ros2 launch ma_lio city02_mapping.launch.py \
  city02_root:=/path/to/City02/sensor_data \
  rate:=1.0
```

---

## 8) Mô tả dataset City02 hiện tại

Cấu trúc đầu vào:
```text
City02/
└── sensor_data/
   ├── ouster/
   ├── Livox_avia/
   ├── Livox_tele/
   ├── xsens_imu.csv
   ├── data_stamp.csv
   └── ouster_stamp.csv
```

- `ouster/*.bin`, `Livox_avia/*.bin`, `Livox_tele/*.bin` được replay thành `sensor_msgs/msg/PointCloud2`.
- `xsens_imu.csv` được replay thành `sensor_msgs/msg/Imu`.
- Topic mặc định:
  - `/ouster_points`
  - `/livox_avia_points`
  - `/livox_tele_points`
  - `/imu/data`

---

## 9) Tồn đọng ROS1 cần theo dõi tiếp (checklist sống)
- [x] `MA_LIO/src/laserMapping.cpp`: đã bỏ `ros::` + `tf::` ROS1, chuyển sang `rclcpp` + `tf2_ros::TransformBroadcaster`.
- [x] `MA_LIO/src/preprocess.*`: đã đổi kiểu `PointCloud2`, `Publisher`, `Time` sang ROS2 (`sensor_msgs::msg`, `rclcpp`).
- [x] `MA_LIO/src/parameters.*`: đã đổi interface đọc param sang `rclcpp::Node` + `declare/get_parameter`.
- [x] `MA_LIO/src/IMU_Processing.hpp`, `MA_LIO/include/common_lib.h`: đã đổi sang `sensor_msgs::msg::Imu::ConstSharedPtr`, bỏ include ROS1 `ros/ros.h` và `tf` trong header dùng chung.
- [ ] `file_player/*`: port toàn bộ từ catkin/rosbag/dynamic_reconfigure sang ROS2 hoặc thay bằng `city02_player_py`.
- [ ] Chốt chiến lược thay thế `livox_ros_driver/CustomMsg` ROS1.

---

## 10) Log tiến độ (2026-05-22)
- ✅ `MA_LIO/src/parameters.*` đã chuyển sang API parameter ROS2 (`rclcpp::Node`).
- ✅ `MA_LIO/src/preprocess.*` đã chuyển các kiểu ROS1 `PointCloud2/Publisher/Time` sang ROS2.
- ✅ `irp_sen_msgs` đã sang ROS2 (ament + rosidl).
- ✅ Đã có player ROS2 cho City02: `city02_player_py`.
- ✅ Đã thêm launch gộp `city02_mapping.launch.py` để chạy player + thuật toán cùng lúc.
- ✅ Đã tiếp tục dọn ROS1 API trong `IMU_Processing.hpp` và `common_lib.h` (đổi kiểu con trỏ IMU + include message ROS2).
- ⚠️ `file_player/*` vẫn còn ROS1 API (catkin/roscpp/rosbag/dynamic_reconfigure) nên toàn repo chưa chuyển đổi hoàn toàn.

- ✅ `MA_LIO/src/laserMapping.cpp` đã đổi sang ROS2 API: `rclcpp::Node`, `rclcpp::Publisher/Subscription`, `rclcpp::Time`, `rclcpp::spin_some`, và TF broadcaster ROS2 (`tf2_ros`).
