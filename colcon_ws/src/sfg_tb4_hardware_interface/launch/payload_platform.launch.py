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
local_namespace = RosFqnBuilder().scope(Scope.Local).agent()
global_namespace = RosFqnBuilder().scope(Scope.Global).agent()


def generate_launch_description():
    camera_back_fqn_builder = global_namespace.component(Component.Camera, "back")
    camera_back_node = ComposableNode(
        package="sfg_depthai",
        plugin="sfg_depthai::Camera",
        namespace=local_namespace.build(RosFqnSegment.Scope, RosFqnSegment.Agent),
        name=camera_back_fqn_builder.build(RosFqnSegment.Component),
        parameters=[
            PathJoinSubstitution(
                [
                    FindPackageShare(package_name),
                    "config",
                    f"{camera_back_fqn_builder.build(RosFqnSegment.Component)}.yaml",
                ],
            ),
            {
                "frame_id": f"{camera_back_fqn_builder.build(RosFqnSegment.Agent, RosFqnSegment.Component)}_color_optical_frame",
            },
        ],
        remappings=[
            (
                RosFqnBuilder()
                .stream(Stream.Depth)
                .resource(Resource.ImageRaw)
                .build(RosFqnSegment.Stream, RosFqnSegment.Resource)
                + "/compressedDepth",
                camera_back_fqn_builder.stream(Stream.Depth)
                .resource(Resource.ImageCompressed)
                .build(),
            ),
            (
                RosFqnBuilder()
                .stream(Stream.Depth)
                .resource(Resource.CameraInfo)
                .build(RosFqnSegment.Stream, RosFqnSegment.Resource),
                camera_back_fqn_builder.stream(Stream.Depth)
                .resource(Resource.CameraInfo)
                .build(),
            ),
            (
                RosFqnBuilder()
                .stream(Stream.Color)
                .resource(Resource.ImageRaw)
                .build(RosFqnSegment.Stream, RosFqnSegment.Resource)
                + "/ffmpeg",
                camera_back_fqn_builder.stream(Stream.Color)
                .resource(Resource.ImageCompressed)
                .build(),
            ),
            (
                RosFqnBuilder()
                .stream(Stream.Color)
                .resource(Resource.CameraInfo)
                .build(RosFqnSegment.Stream, RosFqnSegment.Resource),
                camera_back_fqn_builder.stream(Stream.Color)
                .resource(Resource.CameraInfo)
                .build(),
            ),
        ],
    )

    lidar_right_fqn_builder = global_namespace.component(Component.Lidar, "right")
    lidar_right_node = ComposableNode(
        package="livox_ros_driver2",
        plugin="livox_ros::DriverNode",
        namespace=local_namespace.build(RosFqnSegment.Scope, RosFqnSegment.Agent),
        name=lidar_right_fqn_builder.build(RosFqnSegment.Component),
        parameters=[
            PathJoinSubstitution(
                [
                    FindPackageShare(package_name),
                    "config",
                    f"{lidar_right_fqn_builder.build(RosFqnSegment.Component)}.yaml",
                ]
            ),
            {
                "user_config_path": PathJoinSubstitution(
                    [
                        FindPackageShare(package_name),
                        "config",
                        f"{lidar_right_fqn_builder.build(RosFqnSegment.Component)}.json",
                    ]
                ),
                "point_cloud_frame_id": f"{lidar_right_fqn_builder.build(RosFqnSegment.Agent, RosFqnSegment.Component)}_point_cloud_frame",
                "imu_frame_id": f"{lidar_right_fqn_builder.build(RosFqnSegment.Agent, RosFqnSegment.Component)}_imu_frame",
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
    )

    return launch.LaunchDescription(
        [
            ComposableNodeContainer(
                package="rclcpp_components",
                executable="component_container_mt",
                namespace=local_namespace.build(
                    RosFqnSegment.Scope, RosFqnSegment.Agent
                ),
                name="payload_platform_container",
                output="screen",
                composable_node_descriptions=[
                    camera_back_node,
                    lidar_right_node,
                ],
            ),
        ]
    )
