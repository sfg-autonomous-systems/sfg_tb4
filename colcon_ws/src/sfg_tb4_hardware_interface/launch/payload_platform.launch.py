import launch
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from launch_ros.substitutions import FindPackageShare
from rospkg import get_package_name
from sfg_utils.fqn import (
    Component,
    Resource,
    RosFqnBuilder,
    RosFqnSegment,
    Scope,
    Stream,
)

package_name = get_package_name(__file__)
local_namespace, global_namespace = (
    RosFqnBuilder()
    .scope(Scope.Local)
    .agent()
    .build(begin=RosFqnSegment.Scope, end=RosFqnSegment.Agent),
    RosFqnBuilder()
    .scope(Scope.Global)
    .agent()
    .build(begin=RosFqnSegment.Scope, end=RosFqnSegment.Agent),
)


def generate_launch_description():
    camera_back_fqn_builder = (
        RosFqnBuilder().scope(Scope.Global).agent().component(Component.Camera, "back")
    )
    camera_back_name = camera_back_fqn_builder.build(RosFqnSegment.Component)
    camera_back_node = ComposableNode(
        package="sfg_depthai",
        plugin="sfg_depthai::Camera",
        namespace=local_namespace,
        name=camera_back_name,
        parameters=[
            PathJoinSubstitution(
                [
                    FindPackageShare(package_name),
                    "config",
                    f"{camera_back_name}.yaml",
                ]
            )
        ],
        remappings=[
            (
                "camera/depth/image_raw/compressedDepth",
                camera_back_fqn_builder.stream(Stream.Depth)
                .resource(Resource.ImageCompressed)
                .build(),
            ),
            (
                "camera/depth/camera_info",
                camera_back_fqn_builder.resource(Resource.CameraInfo).build(),
            ),
            (
                "camera/color/image_raw/ffmpeg",
                camera_back_fqn_builder.stream(Stream.Color)
                .resource(Resource.ImageCompressed)
                .build(),
            ),
            (
                "camera/color/camera_info",
                camera_back_fqn_builder.resource(Resource.CameraInfo).build(),
            ),
        ],
    )

    lidar_right_fqn_builder = (
        RosFqnBuilder().scope(Scope.Global).agent().component(Component.Lidar, "right")
    )
    lidar_right_name = lidar_right_fqn_builder.build(RosFqnSegment.Component)
    lidar_right_node = ComposableNode(
        package="livox_ros_driver2",
        plugin="livox_ros::DriverNode",
        namespace=local_namespace,
        name=lidar_right_name,
        parameters=[
            PathJoinSubstitution(
                [
                    FindPackageShare(package_name),
                    "config",
                    f"{lidar_right_name}.yaml",
                ]
            ),
            {
                "user_config_path": PathJoinSubstitution(
                    [
                        FindPackageShare(package_name),
                        "config",
                        f"{lidar_right_name}.json",
                    ]
                ),
                "point_cloud_frame_id": f"{lidar_right_fqn_builder.build(begin=RosFqnSegment.Agent, end=RosFqnSegment.Component)}_point_cloud_frame",
                "imu_frame_id": f"{lidar_right_fqn_builder.build(begin=RosFqnSegment.Agent, end=RosFqnSegment.Component)}_imu_frame",
            },
        ],
        remappings=[
            (
                "livox/imu",
                lidar_right_fqn_builder.resource(Resource.Imu).build(),
            ),
            (
                "livox/lidar",
                lidar_right_fqn_builder.resource(Resource.PointCloud).build(),
            ),
        ],
        extra_arguments=[{"use_intra_process_comms": True}],
    )

    return launch.LaunchDescription(
        [
            ComposableNodeContainer(
                package="rclcpp_components",
                executable="component_container_mt",
                namespace=local_namespace,
                name="payload_platform_container",
                output="screen",
                composable_node_descriptions=[
                    camera_back_node,
                    lidar_right_node,
                ],
            ),
        ]
    )
