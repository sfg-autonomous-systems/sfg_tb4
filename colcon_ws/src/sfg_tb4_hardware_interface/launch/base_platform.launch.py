import launch
from launch.substitutions import Command, PathJoinSubstitution
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


def generate_launch_description() -> launch.LaunchDescription:
    robot_state_publisher_fqn_builder = RosFqnBuilder().scope(Scope.Global).agent()
    robot_state_publisher_name = (
        RosFqnBuilder()
        .component(Component.Custom, "robot_state_publisher")
        .build(RosFqnSegment.Component)
    )
    robot_state_publisher_node = ComposableNode(
        package="robot_state_publisher",
        plugin="robot_state_publisher::RobotStatePublisher",
        namespace=local_namespace,
        name=robot_state_publisher_name,
        parameters=[
            {
                "robot_description": Command(
                    [
                        "xacro ",
                        PathJoinSubstitution(
                            [
                                FindPackageShare("sfg_tb4_description"),
                                "xacro",
                                "robot_description.urdf.xacro",
                            ]
                        ),
                    ]
                ),
                "frame_prefix": f"{robot_state_publisher_fqn_builder.build(RosFqnSegment.Agent)}/",
            }
        ],
        remappings=[
            (
                robot_state_publisher_fqn_builder.resource(
                    Resource.RobotDescription
                ).build(RosFqnSegment.Resource),
                robot_state_publisher_fqn_builder.build(),
            )
        ],
    )

    return launch.LaunchDescription(
        [
            ComposableNodeContainer(
                package="rclcpp_components",
                executable="component_container_mt",
                namespace=local_namespace,
                name="base_platform_container",
                output="screen",
                composable_node_descriptions=[
                    robot_state_publisher_node,
                ],
            ),
        ]
    )
