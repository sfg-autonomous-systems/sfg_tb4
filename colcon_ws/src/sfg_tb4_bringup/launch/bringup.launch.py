import launch
import sfg_utils.launch_utils
from launch.actions import SetLaunchConfiguration
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import ComposableNodeContainer
from launch_ros.substitutions import FindPackageShare
from rospkg import get_package_name
from sfg_utils.fqn import RosFqnBuilder, RosFqnSegment, Scope

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
    agent_launch_description_entities = (
        sfg_utils.launch_utils.get_launch_description_entities(
            "sfg_agent",
            "agent.launch.py",
        )
    )

    hardware_interface_launch_description_entities = (
        sfg_utils.launch_utils.get_launch_description_entities(
            "sfg_tb4_hardware_interface",
            "hardware_interface.launch.py",
        )
    )

    return launch.LaunchDescription(
        [
            SetLaunchConfiguration(
                "metadata_filepath",
                PathJoinSubstitution(
                    [FindPackageShare(package_name), "config", "agent_metadata.yaml"]
                ),
            ),
            *agent_launch_description_entities.launch_arguments,
            *hardware_interface_launch_description_entities.launch_arguments,
            *agent_launch_description_entities.nodes,
            *hardware_interface_launch_description_entities.nodes,
            ComposableNodeContainer(
                package="rclcpp_components",
                executable="component_container_mt",
                namespace=local_namespace,
                name="bringup_container",
                output="screen",
                composable_node_descriptions=agent_launch_description_entities.composable_nodes
                + hardware_interface_launch_description_entities.composable_nodes,
            ),
        ]
    )
