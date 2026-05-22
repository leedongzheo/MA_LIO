#include "parameters.h"

bool path_en = true, scan_pub_en = false, dense_pub_en = false, pcd_save_en = false, time_sync_en = false, extrinsic_est_en = true;
int NUM_MAX_ITERATIONS = 0, lid_num, pcd_save_interval = -1, add_point_size = 0, kdtree_delete_counter = 0, feats_down_size = 0, pcd_index = 0;
int livox_num = 0, spin_num = 0;
std::string map_file_path, imu_topic, root_dir = ROOT_DIR;
std::vector<std::string> lid_topic;
std::vector<int> lid_type, N_SCANS, point_filter_num;
std::vector<double> extrinT, extrinR;
double time_diff_lidar_to_imu = 0.0, filter_size_surf_min = 0, filter_size_map_min = 0, fov_deg = 0, cube_len = 0, range_min, range_max;
double gyr_cov = 0.1, acc_cov = 0.1, b_gyr_cov = 0.0001, b_acc_cov = 0.0001;
float plane_th, DET_RANGE = 300.0f;

double cov_threshold, point_cov_max, point_cov_min, plane_cov_max, plane_cov_min, localize_cov_max, localize_cov_min, localize_thresh_max, localize_thresh_min;
shared_ptr<Preprocess> p_pre;

void readParameters(const rclcpp::Node::SharedPtr &node)
{
  p_pre.reset(new Preprocess());
  auto gp=[&](const std::string& n, auto& v, const auto& d){ node->declare_parameter(n,d); node->get_parameter(n,v); };

  gp("publish.path_en", path_en, true);
  gp("publish.scan_publish_en", scan_pub_en, true);
  gp("publish.dense_publish_en", dense_pub_en, true);
  gp("max_iteration", NUM_MAX_ITERATIONS, 4);
  gp("map_file_path", map_file_path, std::string(""));
  gp("common.lid_topic", lid_topic, std::vector<std::string>{});
  gp("common.imu_topic", imu_topic, std::string("/livox/imu"));
  gp("common.lid_num", lid_num, 2);
  gp("common.lid_type", lid_type, std::vector<int>{});
  gp("common.N_SCANS", N_SCANS, std::vector<int>{});
  gp("common.point_filter_num", point_filter_num, std::vector<int>{});
  gp("common.time_sync_en", time_sync_en, false);
  gp("common.time_offset_lidar_to_imu", time_diff_lidar_to_imu, 0.0);
  gp("filter_size_surf", filter_size_surf_min, 0.5);
  gp("filter_size_map", filter_size_map_min, 0.5);
  gp("cube_side_length", cube_len, 200.0);
  gp("plane_th", plane_th, 0.1f);
  gp("range_min", range_min, 0.0);
  gp("range_max", range_max, 1.0);
  gp("mapping.extrinsic_T", extrinT, std::vector<double>{});
  gp("mapping.extrinsic_R", extrinR, std::vector<double>{});
  gp("mapping.det_range", DET_RANGE, 300.f);
  gp("mapping.fov_degree", fov_deg, 180.0);
  gp("mapping.gyr_cov", gyr_cov, 0.1);
  gp("mapping.acc_cov", acc_cov, 0.1);
  gp("mapping.b_gyr_cov", b_gyr_cov, 0.0001);
  gp("mapping.b_acc_cov", b_acc_cov, 0.0001);
  gp("preprocess.blind", p_pre->blind, 0.01);
  gp("preprocess.timestamp_unit", p_pre->time_unit, US);
  gp("mapping.extrinsic_est_en", extrinsic_est_en, true);
  gp("pcd_save.pcd_save_en", pcd_save_en, false);
  gp("pcd_save.interval", pcd_save_interval, -1);
  gp("cov_threshold", cov_threshold, 0.3);
  gp("uncertainty.point_cov_max", point_cov_max, 0.002);
  gp("uncertainty.point_cov_min", point_cov_min, 0.0005);
  gp("uncertainty.plane_cov_max", plane_cov_max, 1.0);
  gp("uncertainty.plane_cov_min", plane_cov_min, 0.7);
  gp("uncertainty.localize_cov_max", localize_cov_max, 2.0);
  gp("uncertainty.localize_cov_min", localize_cov_min, 0.4);
  gp("uncertainty.localize_thresh_max", localize_thresh_max, 0.8);
  gp("uncertainty.localize_thresh_min", localize_thresh_min, 0.3);
}
