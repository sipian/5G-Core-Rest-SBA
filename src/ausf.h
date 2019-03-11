#ifndef AUSF_H
#define AUSF_H

#include "diameter.h"
#include "gtp.h"
#include "mysql.h"
#include "network.h"
#include "packet.h"
#include "ports.h"
#include "ppconsul/agent.h"
#include "s1ap.h"
#include "sctp_client.h"
#include "sctp_server.h"
#include "sync.h"
#include "utils.h"
#include <jsoncpp/json/json.h>

extern string g_ausf_ip_addr;
extern int g_ausf_port;

extern string g_udm_ip_addr;
extern int g_udm_port;

class Ausf {
private:
	pthread_mutex_t mysql_client_mux;

public:
	SctpServer server;
	MySql mysql_client;

	Ausf();
	void handle_mysql_conn();
	std::string handle_autninfo_req(Json::Value &jsonPkt, int);
	std::string handle_location_update(Json::Value &jsonPkt, int);
	~Ausf();
};

#endif //HSS_H
