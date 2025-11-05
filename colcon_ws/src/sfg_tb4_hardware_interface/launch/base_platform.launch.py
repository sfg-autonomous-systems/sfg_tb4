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
local_namespace = RosFqnBuilder().scope(Scope.Local).agent()
global_namespace = RosFqnBuilder().scope(Scope.Global).agent()


def generate_launch_description() -> launch.LaunchDescription:
    locomotion_controller_fqn_builder = global_namespace.component(
        Component.LocomotionController
    )
    locomotion_controller_node = ComposableNode(
        package=package_name,
        plugin=f"{package_name}::LocomotionController",
        namespace=local_namespace.build(RosFqnSegment.Scope, RosFqnSegment.Agent),
        name=locomotion_controller_fqn_builder.build(RosFqnSegment.Component),
        parameters=[
            PathJoinSubstitution(
                [
                    FindPackageShare(package_name),
                    "config",
                    f"{locomotion_controller_fqn_builder.build(RosFqnSegment.Component)}.yaml",
                ]
            )
        ],
        remappings=[
            (
                RosFqnBuilder().resource(Resource.CmdVel).build(RosFqnSegment.Resource),
                locomotion_controller_fqn_builder.resource(Resource.CmdVel).build(),
            ),
            (
                RosFqnBuilder()
                .resource(Resource.TriggerAction)
                .build(RosFqnSegment.Resource),
                locomotion_controller_fqn_builder.resource(
                    Resource.TriggerAction
                ).build(),
            ),
        ],
    )

    teleop_controller_driver_fqn_builder = global_namespace.component(
        Component.Custom, "teleop_controller_driver"
    )
    teleop_controller_driver_node = Node(
        package="joy_linux",
        executable="joy_linux_node",
        namespace=locomotion_controller_fqn_builder.scope(Scope.Local).build(
            RosFqnSegment.Scope, RosFqnSegment.Component
        ),
        name=teleop_controller_driver_fqn_builder.build(RosFqnSegment.Component),
        parameters=[
            PathJoinSubstitution(
                [
                    FindPackageShare(package_name),
                    "config",
                    f"{teleop_controller_driver_fqn_builder.build(RosFqnSegment.Component)}.yaml",
                ]
            )
        ],
    )

    teleop_controller_fqn_builder = global_namespace.component(
        Component.Custom, "teleop_controller"
    )
    teleop_controller_node = Node(
        package="teleop_twist_joy",
        executable="teleop_node",
        namespace=locomotion_controller_fqn_builder.scope(Scope.Local).build(
            RosFqnSegment.Scope, RosFqnSegment.Component
        ),
        name=teleop_controller_fqn_builder.build(RosFqnSegment.Component),
        parameters=[
            PathJoinSubstitution(
                [
                    FindPackageShare(package_name),
                    "config",
                    f"{teleop_controller_fqn_builder.build(RosFqnSegment.Component)}.yaml",
                ]
            )
        ],
        remappings=[
            (
                RosFqnBuilder().resource(Resource.CmdVel).build(RosFqnSegment.Resource),
                local_namespace.component(Component.Create3)
                .resource(Resource.CmdVel)
                .build(),
            ),
        ],
    )

    robot_state_publisher_fqn_builder = global_namespace.component(
        Component.Custom, "robot_state_publisher"
    )
    robot_state_publisher_node = ComposableNode(
        package="robot_state_publisher",
        plugin="robot_state_publisher::RobotStatePublisher",
        namespace=local_namespace.build(RosFqnSegment.Scope, RosFqnSegment.Agent),
        name=robot_state_publisher_fqn_builder.build(RosFqnSegment.Component),
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
                "frame_prefix": f"{global_namespace.build(RosFqnSegment.Agent)}/",
            }
        ],
        remappings=[
            (
                RosFqnBuilder()
                .resource(Resource.RobotDescription)
                .build(RosFqnSegment.Resource),
                global_namespace.resource(Resource.RobotDescription).build(),
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
                namespace=local_namespace.build(
                    RosFqnSegment.Scope, RosFqnSegment.Agent
                ),
                name="base_platform_container",
                output="screen",
                composable_node_descriptions=[
                    locomotion_controller_node,
                    robot_state_publisher_node,
                ],
            ),
        ]
    )
