#pragma once
#include <string>
#include <QByteArray>

#define BEGIN_HANDLE_PACKET(packet_type) \
  switch (packet_type) { \

#define HANDLE_PACKET(handle_packet, fun) \
  case handle_packet :  \
  {                                       \
      fun(packet_serial, std::move(body)); \
  } \
  break; \

#define END_HANDLE_PACKET() \
  }


class PacketUtil {
 public:
  static int PacketData(bool is_request, int16_t packet_type,
                        int packet_serial,
                        const std::string& source,
                        std::string& head);
  static bool UnPacketData(QByteArray& buffer,
                           bool& is_request,
                           int16_t& packet_type,
                           int& packet_serial,
                           std::string& body);
};
