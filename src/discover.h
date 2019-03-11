#ifndef DISCOVER_H
#define DISCOVER_H

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include "ppconsul/agent.h"
#include "utils.h"

#define CONSUL_ADDR "192.168.116.222:8500"
#define INTERFACE_NAME "eth0"

string getMyIPAddress(string interface = INTERFACE_NAME);
void serviceRegister(string serviceName, ppconsul::agent::Agent &consulAgent);
string serviceDiscovery(string host, ppconsul::agent::Agent& consulAgent);
string resolve_host(string host);

#endif /* DISCOVER_H */
