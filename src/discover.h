//#include <iostream>
#include <netdb.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>


string resolve_host(string host) {
  hostent * record = gethostbyname(host.c_str());
  if (record == NULL) {
    printf("%s is unknown\n", host.c_str());
    return "";
  }
  in_addr * address = (in_addr * )record->h_addr;
  return inet_ntoa(* address);
}
