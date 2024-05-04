#include "crsf_receiver.h"


CrsfReceiverNode::CrsfReceiverNode(): Node("crsf_reader_node")
{
    this->declare_parameter("device", "/dev/ttyUSB0");
    this->declare_parameter("baudrate", CRSF_BAUDRATE);
    this->declare_parameter("link_stats", true);
    this->declare_parameter("receiver_rate", 100);

    channels_publisher = this->create_publisher<crsf_receiver_msg::msg::CRSFChannels16>(
        "rc/channels", 
        rclcpp::QoS(1).best_effort().durability_volatile()
    );

    link_publisher = this->create_publisher<crsf_receiver_msg::msg::CRSFLinkInfo>(
        "rc/link", 
        rclcpp::QoS(1).best_effort().durability_volatile()
    );

    device = this->get_parameter("device").as_string();
    int baudrate = this->get_parameter("baudrate").as_int();
    int rate = this->get_parameter("receiver_rate").as_int();
    int period = 1000 / rate;

    RCLCPP_INFO(this->get_logger(), "Receiver rate is %dhz (period %dms)", rate, period);
    RCLCPP_INFO(this->get_logger(), "Target serial device is: %s", device.c_str());
    RCLCPP_INFO(this->get_logger(), "Selected baudrate: %d", baudrate);

    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(period), 
        std::bind(&CrsfReceiverNode::main_timer_callback, this)
    );
    
    timer_tele_ = this->create_wall_timer(
        std::chrono::milliseconds(100), 
        std::bind(&CrsfReceiverNode::tele_timer_callback, this)
    );

    serial.SetDevice(device);
    serial.SetBaudRate(baudrate);
    serial.SetTimeout(period / 2);
}

void CrsfReceiverNode::main_timer_callback()
{
    if(serial.GetState() == CppLinuxSerial::State::CLOSED) {
        try {
            serial.Open();
            
        } catch(const CppLinuxSerial::Exception& e) {
            RCLCPP_WARN(this->get_logger(), "Can not open serial port: %s", device.c_str());
            return;
        }
    }

    if(serial.Available())
    {
        serial.ReadBinary(parser.rx_buffer);
        parser.parse_incoming_bytes();
    }

    if(parser.is_channels_actual()) {
        CRSFChannels16 message = convert_to_channels_message(parser.get_channels_values());
        channels_publisher->publish(message);
    }

    if(parser.is_link_statistics_actual()) {
        CRSFLinkInfo message = convert_to_link_info(parser.get_link_info());
        link_publisher->publish(message);
    }
}

void CrsfReceiverNode::tele_timer_callback()
{
    // 
    // C8 0A 08 00 4A 00 0A 00 27 10 46 DE 
    // 回传数据 电压7.4V 电流1.0A 容量10000mAh 剩余70%
    std::vector<uint8_t> tx_buffer = {0xC8, 0x0A, 0x08, 0x00, 0x4A, 0x00, 0x0A, 0x00, 0x27, 0x10, 0x46, 0xDE};
    serial.WriteBinary(tx_buffer);
}

CrsfReceiverNode::~CrsfReceiverNode() {
    if(serial.GetState() == CppLinuxSerial::State::OPEN) {
        serial.Close();
    }
}
