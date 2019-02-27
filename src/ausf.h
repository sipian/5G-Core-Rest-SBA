#ifndef AUSF_H
#define AUSF_H

#include "diameter.h"
#include "gtp.h"
#include "mysql.h"
#include "network.h"
#include "packet.h"
#include "s1ap.h"
#include "sctp_client.h"
#include "sctp_server.h"
#include "sync.h"
#include "utils.h"

extern string g_ausf_ip_addr;
extern int g_ausf_port;

extern string g_udm_ip_addr;
extern int g_udm_port;

class RestPacket
{
	public:
	uint64_t imsi;
	uint64_t plmn_id;
	uint64_t autn_vector;
	uint64_t nw_type;
	uint64_t xres;
	uint64_t k_asme;
	uint64_t rand_num;
	uint64_t autn_num;
	uint64_t ksi_asme;
};


class Ausf {
private:
	pthread_mutex_t mysql_client_mux;
	
public:
	SctpServer server;
	MySql mysql_client;

	Ausf();
	void handle_mysql_conn();
	void handle_autninfo_req(RestPacket&, SctpClient&);
	void handle_location_update(int, Packet&, SctpClient&);
	~Ausf();
};

#endif //HSS_H
