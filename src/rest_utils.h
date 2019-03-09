#ifndef REST_UTILS_H
#define REST_UTILS_H

#include <string>
#include <jsoncpp/json/json.h>

bool send_and_receive(std::string ip_addr, int port, std::string route, Json::Value &sendPkt, Json::Value &recvPkt);

#endif /* REST_UTILS_H */
