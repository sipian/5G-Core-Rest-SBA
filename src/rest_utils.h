#ifndef REST_UTILS_H
#define REST_UTILS_H

#include <string>
#include <jsoncpp/json/json.h>

bool send_and_receive(std::string ip_addr, int port, std::string route, Json::Value &sendPkt, Json::Value &recvPkt);

Json::Value touint64(uint64_t val);
Json::Value touint(uint32_t val);
Json::Value touint(uint16_t val);
Json::Value touint(uint8_t val);
Json::Value toint(int val);
Json::Value tobool(bool val);

#endif /* REST_UTILS_H */
