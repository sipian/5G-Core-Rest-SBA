#include <ifaddrs.h>
#include <netdb.h>
#include "ppconsul/agent.h"
#include "utils.h"

#define CONSUL_ADDR "192.168.116.222:8500"
#define INTERFACE_NAME "eth0"

string getMyIPAddress(string interface = INTERFACE_NAME) {
    struct ifaddrs * ifAddrStruct = NULL;
    struct ifaddrs * ifa = NULL;
    void * tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);

    string address = "";

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if(strcasecmp(ifa->ifa_name, interface.c_str()) == 0) {
              address = string(addressBuffer);
              break;
            }
        }
    }
    if (ifAddrStruct!=NULL) {
      freeifaddrs(ifAddrStruct);
    }
    return address;
}


void serviceRegister(string serviceName, ppconsul::agent::Agent &consulAgent)
{
  string nodeIPAddress = getMyIPAddress();
  try
  {
      consulAgent.registerService(
          ppconsul::agent::kw::id = serviceName,
          ppconsul::agent::kw::name = serviceName,
          ppconsul::agent::kw::address = nodeIPAddress
      );
      TRACE(cout << "[INFO] :: ServiceRegister :: {" << serviceName << "} - Registered successfully with consul with Address " << nodeIPAddress << ".\n";)
      return;
  }
  catch (const std::runtime_error &re)
  {
      // specific handling for runtime_error
      cout << "[ERROR] :: ServiceRegister :: {" << serviceName << "} - Runtime error :: " << re.what() << ".\n";
  }
  catch (const std::exception &ex)
  {
      // specific handling for all exceptions extending std::exception, except
      // std::runtime_error which is handled explicitly
      cout << "[ERROR] :: ServiceRegister :: {" << serviceName << "} - Exception :: " << ex.what() << ".\n";
  }
  catch (...)
  {
      // catch any other errors (that we have no information about)
      cout << "[ERROR] :: ServiceRegister :: {" << serviceName << "} - Exception :: Unknown error occurred.\n";
  }
  exit(1);
}

string serviceDiscover(string host, ppconsul::agent::Agent& consulAgent) {
  auto services =  consulAgent.services();
  string addr = services[host].address;

  if (addr == "") {
    cout << "[ERROR] :: serviceDiscover - Could not resolve " << host << ".\n";
    exit(0);
  }
  return addr;
}

string resolve_host(string host) {
  hostent * record = gethostbyname(host.c_str());
  if (record == NULL) {
    cout << host << " is unknown\n";
    return "";
  }
  in_addr * address = (in_addr * )record->h_addr;
  return inet_ntoa(* address);
}
