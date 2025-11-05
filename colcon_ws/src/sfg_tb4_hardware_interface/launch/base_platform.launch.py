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


def generate_launch_description() -> launch.LaunchDescription:
    locomotion_controller_name = (
        RosFqnBuilder()
        .component(Component.LocomotionController)
        .build(RosFqnSegment.Component)
    )
    locomotion_controller_node = ComposableNode(
        package=package_name,
        plugin=f"{package_name}::LocomotionController",
        namespace=RosFqnBuilder()
        .scope(Scope.Local)
        .agent()
        .build(RosFqnSegment.Scope, RosFqnSegment.Agent),
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
                RosFqnBuilder().resource(Resource.CmdVel).build(RosFqnSegment.Resource),
                RosFqnBuilder()
                .scope(Scope.Global)
                .agent()
                .component(Component.LocomotionController)
                .resource(Resource.CmdVel)
                .build(),
            ),
            (
                RosFqnBuilder()
                .resource(Resource.TriggerAction)
                .build(RosFqnSegment.Resource),
                RosFqnBuilder()
                .scope(Scope.Global)
                .agent()
                .component(Component.LocomotionController)
                .resource(Resource.TriggerAction)
                .build(),
            ),
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
        namespace=RosFqnBuilder()
        .scope(Scope.Local)
        .agent()
        .component(Component.LocomotionController)
        .build(RosFqnSegment.Scope, RosFqnSegment.Component),
        name=teleop_controller_driver_name,
        parameters=[
            PathJoinSubstitution(
                [
                    FindPackageShare(package_name),
                    "config",
                    f"{teleop_controller_driver_name}.yaml",
                ]
            )
        ],
    )

    teleop_controller_name = (
        RosFqnBuilder()
        .component(Component.Custom, "teleop_controller")
        .build(RosFqnSegment.Component)
    )
    teleop_controller_node = Node(
        package="teleop_twist_joy",
        executable="teleop_node",
        namespace=RosFqnBuilder()
        .scope(Scope.Local)
        .agent()
        .component(Component.LocomotionController)
        .build(RosFqnSegment.Scope, RosFqnSegment.Component),
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
                RosFqnBuilder().resource(Resource.CmdVel).build(RosFqnSegment.Resource),
                RosFqnBuilder()
                .scope(Scope.Local)
                .agent()
                .component(Component.Create3)
                .resource(Resource.CmdVel)
                .build(),
            ),
        ],
    )

    robot_state_publisher_fqn_builder = RosFqnBuilder().scope(Scope.Global).agent()
    robot_state_publisher_name = (
        RosFqnBuilder()
        .component(Component.Custom, "robot_state_publisher")
        .build(RosFqnSegment.Component)
    )
    robot_state_publisher_node = ComposableNode(
        package="robot_state_publisher",
        plugin="robot_state_publisher::RobotStatePublisher",
        namespace=RosFqnBuilder()
        .scope(Scope.Local)
        .agent()
        .build(RosFqnSegment.Scope, RosFqnSegment.Agent),
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
            teleop_controller_driver_node,
            teleop_controller_node,
            ComposableNodeContainer(
                package="rclcpp_components",
                executable="component_container_mt",
                namespace=RosFqnBuilder()
                .scope(Scope.Local)
                .agent()
                .build(RosFqnSegment.Scope, RosFqnSegment.Agent),
                name="base_platform_container",
                output="screen",
                composable_node_descriptions=[
                    locomotion_controller_node,
                    robot_state_publisher_node,
                ],
            ),
        ]
    )
