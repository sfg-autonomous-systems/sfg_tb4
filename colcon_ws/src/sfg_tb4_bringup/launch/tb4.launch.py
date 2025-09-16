from pathlib import Path

import launch
from ament_index_python.packages import get_package_share_directory
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import ComposableNodeContainer, Node
from launch_ros.descriptions import ComposableNode
from launch_ros.substitutions import FindPackageShare
from sfg_utils import get_agent_name, sanitize_agent_name

package_directory = Path(get_package_share_directory("sfg_tb4_bringup"))
sanitized_hostname = sanitize_agent_name(get_agent_name())
local_namespace = "/local"
global_namespace = "/global/" + sanitized_hostname


def generate_launch_description():
    jtop_launch_description = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [
                PathJoinSubstitution(
                    [
                        FindPackageShare("isaac_ros_jetson_stats"),
                        "launch",
                        "jtop.launch.py",
                    ]
                )
            ]
        ),
    )

    tb4_container = ComposableNodeContainer(
        package="rclcpp_components",
        executable="component_container_mt",
        namespace=local_namespace,
        name="tb4_container",
        composable_node_descriptions=(
            ComposableNode(
                package="sfg_agent",
                plugin="sfg_agent::AgentStatusProvider",
                namespace=local_namespace,
                parameters=[
                    {
                        "metadata_filepath": (
                            package_directory / "config" / "agent_metadata.yaml"
                        ).as_posix(),
                    },
                ],
                extra_arguments=[{"use_intra_process_comms": True}],
            ),
            ComposableNode(
                package="sfg_depthai",
                plugin="sfg_depthai::Camera",
                namespace=local_namespace,
                name="camera_head",
                parameters=[
                    package_directory / "config" / "camera_head.yaml",
                ],
                remappings=[
                    (
                        "camera_head/depth/image_raw/compressedDepth",
                        f"{global_namespace}/camera_head/depth/image_compressed"",
                    ),
                    (
                        "camera_head/depth/camera_info",
                        f"{global_namespace}/camera_head/depth/camera_info",
                    ),
                    (
                        "camera_head/color/image_raw/ffmpeg",
                        f"{global_namespace}/camera_head/color/image_compressed",
                    ),
                    (
                        "camera_head/color/camera_info",
                        f"{global_namespace}/camera_head/color/camera_info",
                    ),
                ],
            ),
            ComposableNode(
                package="livox_ros_driver2",
                plugin="livox_ros::DriverNode",
                namespace=local_namespace,
                name="lidar_back",
                parameters=[
                    package_directory / "config" / "lidar_back.yaml",
                    {
                        "user_config_path": (
                            package_directory / "config" / "lidar_back_config.json"
                        ).as_posix(),
                    },
                ],
                remappings=[
                    ("livox/imu", f"{global_namespace}/lidar_back/imu"),
                    ("livox/lidar", f"{global_namespace}/lidar_back/points"),
                ],
            ),
        ),
        output="screen",
    )

    return launch.LaunchDescription(
        [
            jtop_launch_description,
            tb4_container,
        ]
    )
