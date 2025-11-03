import launch
from launch.substitutions import Command, PathJoinSubstitution
from launch_ros.actions import ComposableNodeContainer, Node
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
    locomotion_controller_fqn_builder = (
        RosFqnBuilder()
        .scope(Scope.Global)
        .agent()
        .component(Component.LocomotionController)
    )
    locomotion_controller_name = locomotion_controller_fqn_builder.build(
        RosFqnSegment.Component
    )
    locomotion_controller_node = ComposableNode(
        package=package_name,
        plugin=f"{package_name}::LocomotionController",
        namespace=local_namespace,
        name=locomotion_controller_name,
        parameters=[
            PathJoinSubstitution(
                [
                    FindPackageShare(package_name),
                    "config",
                    f"{locomotion_controller_name}.yaml",
                ]
            )
        ],
        remappings=[
            (
                locomotion_controller_fqn_builder.resource(Resource.CmdVel).build(
                    RosFqnSegment.Resource
                ),
                locomotion_controller_fqn_builder.build(),
            ),
            (
                locomotion_controller_fqn_builder.resource(
                    Resource.TriggerAction
                ).build(RosFqnSegment.Resource),
                locomotion_controller_fqn_builder.build(),
            ),
        ],
    )

    robot_state_publisher_fqn_builder = RosFqnBuilder().scope(Scope.Global).agent()
    robot_state_publisher_name = (
        RosFqnBuilder()
        .component(Component.RobotStatePublisher)
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

    teleop_controller_driver_name = (
        RosFqnBuilder()
        .component(Component.Custom, "teleop_controller_driver")
        .build(RosFqnSegment.Component)
    )
    teleop_controller_driver_node = Node(
        package="joy_linux",
        executable="joy_linux_node",
        parameters=[{"dev": "/dev/input/js0"}],
        name=teleop_controller_driver_name,
        remappings=[("/diagnostics", "diagnostics")],
    )

    teleop_controller_name = (
        RosFqnBuilder()
        .component(Component.Custom, "teleop_controller")
        .build(RosFqnSegment.Component)
    )
    teleop_controller_node = Node(
        package="teleop_twist_joy",
        executable="teleop_node",
        name=teleop_controller_name,
        parameters=[
            PathJoinSubstitution(
                [
                    FindPackageShare(package_name),
                    "config",
                    f"{teleop_controller_name}.yaml",
                ]
            )
        ],
        remappings=[
            (
                "/cmd_vel",
                RosFqnBuilder()
                .scope(Scope.Local)
                .agent()
                .component(Component.Create3)
                .resource(Resource.CmdVel)
                .build(),
            ),
        ],
    )

    return launch.LaunchDescription(
        [
            teleop_controller_driver_node,
            teleop_controller_node,
            ComposableNodeContainer(
                package="rclcpp_components",
                executable="component_container_mt",
                namespace=local_namespace,
                name="base_platform_container",
                output="screen",
                composable_node_descriptions=[
                    locomotion_controller_node,
                    robot_state_publisher_node,
                ],
            ),
        ]
    )
