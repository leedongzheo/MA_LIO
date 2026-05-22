from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    city02_root = LaunchConfiguration('city02_root')
    rate = LaunchConfiguration('rate')

    city_yaml = PathJoinSubstitution([
        FindPackageShare('ma_lio'),
        'config',
        'City.yaml',
    ])

    return LaunchDescription([
        DeclareLaunchArgument('city02_root', description='Path to City02/sensor_data'),
        DeclareLaunchArgument('rate', default_value='1.0'),
        Node(
            package='city02_player_py',
            executable='city02_player_node',
            name='city02_player_node',
            output='screen',
            parameters=[{
                'root': city02_root,
                'rate': rate,
            }],
        ),
        Node(
            package='ma_lio',
            executable='malio_mapping',
            name='ma_lio_mapping',
            output='screen',
            parameters=[city_yaml],
        ),
    ])
