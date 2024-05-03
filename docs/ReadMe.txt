$ colcon build
$ source install/setup.bash
$ ros2 run crsf_receiver crsf_receiver_node # baud_rate = 420000

# Check channels values
ros2 topic echo /rc/channels

# Check link statisics
ros2 topic echo /rc/link


