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

## 6) Cách build package MA_LIO trên ROS2 Jazzy

### 6.1 Cài dependency cơ bản
```bash
sudo apt update
sudo apt install -y \
  ros-jazzy-desktop \
  ros-jazzy-pcl-conversions \
  ros-jazzy-pcl-msgs \
  ros-jazzy-tf2-ros \
  ros-jazzy-message-filters \
  libpcl-dev libeigen3-dev
```

### 6.2 Build workspace
Giả sử bạn đặt repo ở workspace ROS2:
```bash
source /opt/ros/jazzy/setup.bash
cd <ros2_ws>
colcon build --symlink-install --packages-select ma_lio
source install/setup.bash
```

> Nếu build fail do API ROS1 còn sót: xem mục 4 để migrate tiếp từng file.

---

## 7) Chạy launch thuật toán trên ROS2
```bash
source /opt/ros/jazzy/setup.bash
source <ros2_ws>/install/setup.bash
ros2 launch ma_lio mapping_city.launch.py
```

---

## 8) Phát dataset City02 bằng ROS2 (không dùng rosbag có sẵn)

Vì dataset hiện là file nhị phân + CSV timestamp, chưa phải `.db3` rosbag2, cách nhanh nhất là viết **publisher node** phát:
- `/ouster_points` từ `City02/sensor_data/ouster/*.bin` + `ouster_stamp.csv`
- `/livox_avia_points` từ `City02/sensor_data/Livox_avia/*.bin` + `data_stamp.csv`
- `/livox_tele_points` từ `City02/sensor_data/Livox_tele/*.bin` + `data_stamp.csv`
- `/imu/data` từ `xsens_imu.csv`

### 8.1 Luồng node publisher khuyến nghị
1. Parse CSV timestamp.
2. Sắp event theo thời gian toàn cục.
3. Theo clock thực (`rclcpp::Clock`) phát tuần tự event.
4. Convert `.bin` -> `sensor_msgs::msg::PointCloud2` (XYZI).
5. Convert IMU CSV -> `sensor_msgs::msg::Imu`.

### 8.2 Khung lệnh chạy
```bash
# terminal 1
ros2 run <dataset_player_pkg> city02_player_node \
  --ros-args \
  -p root:=/path/to/City02/sensor_data \
  -p rate:=1.0

# terminal 2
ros2 launch ma_lio mapping_city.launch.py
```

---

## 9) Khuyến nghị hoàn tất migration (giai đoạn 2)
1. Refactor `laserMapping.cpp` thành class node ROS2 (`MaLioMappingNode`).
2. Chuyển toàn bộ tf ROS1 sang tf2_ros.
3. Chuẩn hóa message input LiDAR về `sensor_msgs/msg/PointCloud2` để bỏ phụ thuộc mạnh vào ROS1 livox msg.
4. Update `preprocess.*` callback + publish API ROS2.
5. Chạy test replay City02 (rate 0.5x -> 1.0x -> 2.0x).
6. Xuất checklist pass/fail từng topic (Hz, frame_id, timestamp monotonic).

---

## 10) Trạng thái hiện tại
- [x] Có tài liệu migration theo dõi tiến trình.
- [x] Có khung build ROS2 (CMake/package/launch).
- [ ] Chưa migrate xong toàn bộ source ROS1 API.
- [ ] Chưa có node player City02 trong repo (mới hướng dẫn kiến trúc chạy).

## 10) Cập nhật tiến trình (2026-05-22)

### Đánh giá nhanh trạng thái ROS2 Jazzy
- **Chưa chuyển hoàn toàn sang ROS2 Jazzy**.
- Repository hiện ở trạng thái **hybrid**: một phần đã có ROS2 metadata (ví dụ `MA_LIO/CMakeLists.txt`, `MA_LIO/package.xml`, launch `.launch.py`) nhưng vẫn còn nhiều mã ROS1 API trong source code runtime.

### Các phần đã xử lý trong lượt này
- ✅ Đã chuyển package message `irp_sen_msgs` từ ROS1/catkin sang ROS2/ament + rosidl:
  - `irp_sen_msgs/CMakeLists.txt`
  - `irp_sen_msgs/package.xml`

### Các phần vẫn còn ROS1 API (chưa hoàn tất chuyển đổi)
1. **`MA_LIO/src/laserMapping.cpp`**
   - Còn dùng: `#include <ros/ros.h>`, `ros::NodeHandle`, `ros::Publisher`, `ros::Subscriber`, `ros::Rate`, `ros::spinOnce`, `ros::Time`, `tf::TransformBroadcaster`, ROS1 message types/includes.
2. **`MA_LIO/src/preprocess.h/.cpp`**
   - Còn dùng `ros::Publisher`, `ros::Time`, kiểu callback ROS1 (`sensor_msgs::PointCloud2::ConstPtr`).
3. **`MA_LIO/src/parameters.h/.cpp`**
   - Còn interface `readParameters(ros::NodeHandle&)`.
4. **`MA_LIO/src/IMU_Processing.hpp`, `MA_LIO/include/common_lib.h`**
   - Còn dùng kiểu ROS1 message pointer (`sensor_msgs::ImuConstPtr`, `sensor_msgs::Imu::ConstPtr`).
5. **Package `file_player`**
   - Còn nguyên ROS1 stack (`catkin`, `dynamic_reconfigure`, `ros::...`, `rosbag`, `.launch` ROS1).

### Kết luận tiến độ hiện tại
- Chuyển đổi ROS2 Jazzy **mới hoàn tất cho `irp_sen_msgs`** trong lượt này.
- Phần runtime chính (`MA_LIO`) và tool (`file_player`) **vẫn cần port tiếp** để đạt mục tiêu “hoàn toàn ROS2”.


## 11) Cập nhật tiến trình (2026-05-22, lượt tiếp theo)

### 11.1 Đã bổ sung ROS2 dataset player cho City02
Đã thêm package mới `city02_player_py` (ROS2 Jazzy, `ament_python`) để replay trực tiếp cấu trúc:

```text
City02/sensor_data/
├── ouster/*.bin
├── Livox_avia/*.bin
├── Livox_tele/*.bin
├── xsens_imu.csv
├── data_stamp.csv
└── ouster_stamp.csv
```

Node `city02_player_node` publish các topic:
- `/ouster_points` (`sensor_msgs/msg/PointCloud2`)
- `/livox_avia_points` (`sensor_msgs/msg/PointCloud2`)
- `/livox_tele_points` (`sensor_msgs/msg/PointCloud2`)
- `/imu/data` (`sensor_msgs/msg/Imu`)

Có launch file ROS2:
- `ros2 launch city02_player_py city02_player.launch.py root:=/abs/path/City02/sensor_data rate:=1.0`

### 11.2 Cách build toàn bộ để chạy MA_LIO + player
```bash
source /opt/ros/jazzy/setup.bash
cd <ros2_ws>
colcon build --symlink-install --packages-select irp_sen_msgs ma_lio city02_player_py
source install/setup.bash
```

### 11.3 Chạy thực nghiệm City02 trên ROS2
Terminal 1 (phát dataset):
```bash
source /opt/ros/jazzy/setup.bash
source <ros2_ws>/install/setup.bash
ros2 launch city02_player_py city02_player.launch.py \
  root:=/abs/path/City02/sensor_data rate:=1.0
```

Terminal 2 (MA_LIO):
```bash
source /opt/ros/jazzy/setup.bash
source <ros2_ws>/install/setup.bash
ros2 launch ma_lio mapping_city.launch.py
```

### 11.4 Tồn đọng ROS1 API (vẫn còn, cần port tiếp)
Các API ROS1 vẫn xuất hiện trong runtime chính `MA_LIO`:
- `ros::init`, `ros::NodeHandle`, `ros::Publisher`, `ros::Subscriber`
- `ros::spinOnce`, `ros::Rate`, `ros::Time`
- `ROS_INFO`, `ROS_WARN`
- `tf::TransformBroadcaster` (tf ROS1)
- callback kiểu `ConstPtr` ROS1 trong nhiều file (`laserMapping.cpp`, `preprocess.*`, `parameters.*`, `IMU_Processing.hpp`)

### 11.5 Gợi ý bước kế tiếp
1. Port `laserMapping.cpp` thành class node ROS2 (`rclcpp::Node`).
2. Thay `tf` ROS1 bằng `tf2_ros::TransformBroadcaster` + `geometry_msgs/msg/TransformStamped`.
3. Chuyển callback và publisher/subscriber ở `preprocess.*` sang ROS2 signatures.
4. Đổi `readParameters(ros::NodeHandle&)` sang đọc tham số kiểu ROS2 (`declare/get_parameter`).
5. Chuẩn hóa đầu vào LiDAR về `sensor_msgs/msg/PointCloud2` để tránh phụ thuộc `livox_ros_driver/CustomMsg` ROS1.
