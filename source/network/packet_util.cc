#include "packet_util.h"
#ifdef OS_WIN
  #include<windows.h>
#else
 #include<arpa/inet.h> 
#endif  // OS_WIN


#define HEAD_SIZE 11
int PacketUtil::PacketData(bool is_request,
                           int16_t packet_type,
                           int packet_serial,
                           const std::string& source,
                           std::string& head) {
  //HEAD_SIZE bytes head
  head.resize(HEAD_SIZE);
  char* pbase = head.data();
  //is request
  memcpy(pbase, &is_request, sizeof(is_request));
  pbase++;
  //packet type
  packet_type = htons(packet_type);
  memcpy(pbase, &packet_type, sizeof(packet_type));
  pbase += sizeof(packet_type);
  //packet serial
  packet_serial = htonl(packet_serial);
  memcpy(pbase, &packet_serial, sizeof(packet_serial));
  pbase += sizeof(packet_serial);
  //size
  int source_len = static_cast<int>(source.size());
  source_len = htonl(source_len);
  memcpy(pbase, &source_len, sizeof(source_len));
  return ntohl(packet_serial);
}

bool PacketUtil::UnPacketData(QByteArray& buffer,
                              bool& is_request,
                              int16_t& packet_type,                            
                              int& packet_serial,
                              std::string& body) {
  int buffer_len = (int)buffer.size();
  // check head 
  if (buffer_len < HEAD_SIZE) {
    return false;
  }
  char* pbase = buffer.data();
  //is_request
  memcpy(&is_request, pbase, sizeof(is_request));
  pbase++;
  //pcket type
  memcpy(&packet_type, pbase, sizeof(packet_type));
  packet_type = ntohs(packet_type);
  pbase += sizeof(packet_type);
  // packet_serial
  memcpy(&packet_serial, pbase, sizeof(packet_serial));
  packet_serial = ntohl(packet_serial);
  pbase += sizeof(packet_serial);

  int source_len = 0;
  memcpy(&source_len, pbase, sizeof(source_len));
  source_len = ntohl(source_len);
  //check body
  if (buffer_len < (HEAD_SIZE + source_len)) {
    return false;
  }
  body = std::string(buffer.data() + HEAD_SIZE, source_len);
  buffer.remove(0, (HEAD_SIZE + source_len));
  return true;
}
