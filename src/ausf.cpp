#include "ausf.h"
#include "discover.h"

using namespace ppconsul;
using ppconsul::Consul;

string g_ausf_ip_addr = "";
string g_udm_ip_addr = "";
int g_ausf_port = G_AUSF_PORT;
int g_udm_port = G_UDM_PORT;

Ausf::Ausf() {
	g_sync.mux_init(mysql_client_mux);

	Consul consul(CONSUL_ADDR);
	agent::Agent consulAgent(consul);

	serviceRegister("ausf", consulAgent);

	g_ausf_ip_addr = getMyIPAddress();
	g_udm_ip_addr = serviceDiscover("udm", consulAgent);
}



void Ausf::handle_mysql_conn() {
	/* Lock not necessary since this is called only once per object. Added for uniformity in locking */
	g_sync.mlock(mysql_client_mux);
	mysql_client.conn();
	g_sync.munlock(mysql_client_mux);
}

void Ausf::handle_autninfo_req(RestPacket &requestPkt, SctpClient &udm_client) {
	uint64_t imsi;
	uint64_t key;
	uint64_t rand_num;
	uint64_t autn_num;
	uint64_t sqn;
	uint64_t xres;
	uint64_t ck;
	uint64_t ik;
	uint64_t k_asme;
	uint64_t num_autn_vectors;
	uint64_t plmn_id;
	uint64_t nw_type;

	imsi = requestPkt.imsi;
	plmn_id = requestPkt.plmn_id;
	num_autn_vectors = requestPkt.autn_vector;
	nw_type = requestPkt.nw_type;

	Packet pkt;
	pkt.clear_pkt();
	pkt.append_item(imsi);
	pkt.prepend_diameter_hdr(1,pkt.len);
	udm_client.snd(pkt);
	// TRACE(cout<<"Packet send to udm"<<endl;)

	// get_autn_info(imsi, key, rand_num);   // will send this request(imsi) for get_auth_info to the udm.
	udm_client.rcv(pkt);
	// TRACE(cout<<"Response recieved from udm"<<endl;)
	pkt.extract_diameter_hdr();
	pkt.extract_item(key);
	pkt.extract_item(rand_num);
	// TRACE(cout << "ausf_handleautoinforeq:" << " retrieved from database: " << imsi << endl;)
	sqn = rand_num + 1;
	xres = key + sqn + rand_num;
	autn_num = xres + 1;
	ck = xres + 2;
	ik = xres + 3;
	k_asme = ck + ik + sqn + plmn_id;
	
	requestPkt.autn_num = autn_num;
	requestPkt.rand_num = rand_num;
	requestPkt.xres = xres;
	requestPkt.k_asme = k_asme;

	std::cout<<"requestPkt.autn_num is (ausf.cpp) "<<requestPkt.autn_num<<endl;
	// pkt.prepend_diameter_hdr(1, pkt.len);
	// server.snd(conn_fd, pkt);
	// TRACE(cout << "ausf_handleautoinforeq:" << " response sent to amf: " << imsi << endl;)
	//return rpkt;
}

void Ausf::handle_location_update(int conn_fd, Packet &pkt, SctpClient &udm_client) {
	uint64_t imsi;
	uint64_t default_apn;
	uint32_t mmei;

	default_apn = 1;
	pkt.extract_item(imsi);
	pkt.extract_item(mmei);
	// set_loc_info(imsi, mmei);
	TRACE(cout<<"Sending out the packet to the udm"<<endl;)
	pkt.clear_pkt();
	pkt.append_item(imsi);
	pkt.append_item(mmei);
	pkt.prepend_diameter_hdr(2,pkt.len);
	udm_client.snd(pkt);
	TRACE(cout<<"Packet sent to udm"<<endl;)
	TRACE(cout << "ausf_handleautoinforeq:" << " loc updated" << endl;)
	pkt.clear_pkt();
	pkt.append_item(default_apn);
	pkt.prepend_diameter_hdr(2, pkt.len);
	server.snd(conn_fd, pkt);
	TRACE(cout << "ausf_handleautoinforeq:" << " loc update complete sent to amf" << endl;)
}

Ausf::~Ausf() {

}
