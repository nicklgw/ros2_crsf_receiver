注意:串口芯片需选用FT232的串口，经测试其他CH340&PL2312芯片均不能正常工作

CRSF(ELRS)协议文档
https://github.com/crsf-wg/crsf/wiki/Packet-Types
https://github.com/crsf-wg/crsf/wiki

该示例已调通，遥测回传已调通
https://github.com/nicklgw/ros2_crsf_receiver.git


$ colcon build
$ source install/setup.bash
$ ros2 run crsf_receiver crsf_receiver_node # baud_rate = 420000

# Check channels values
ros2 topic echo /rc/channels

# Check link statisics
ros2 topic echo /rc/link
