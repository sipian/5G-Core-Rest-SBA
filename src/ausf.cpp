#include "ausf.h"
#include "discover.h"
#include "ports.h"
#include "rest_utils.h"

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
	g_udm_ip_addr = serviceDiscovery("udm", consulAgent);
}

void Ausf::handle_mysql_conn() {
	/* Lock not necessary since this is called only once per object. Added for uniformity in locking */
	g_sync.mlock(mysql_client_mux);
	mysql_client.conn();
	g_sync.munlock(mysql_client_mux);
}

std::string Ausf::handle_autninfo_req(Json::Value &jsonPkt, int worker_id) {
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

	imsi = jsonPkt["imsi"].asUInt64();
	plmn_id = jsonPkt["plmn_id"].asUInt64();
	num_autn_vectors = jsonPkt["num_autn_vectors"].asUInt64();
	nw_type = jsonPkt["nw_type"].asUInt64();

	Json::Value reqPkt, jsonRes;
	reqPkt["imsi"] = touint64(imsi);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr, 
		UDM_PORT_START_RANGE + worker_id, 
		"/Nudm_UECM/GetAuthInfo",
		reqPkt, jsonRes
	);

	key = jsonRes["key"].asUInt64();
	rand_num = jsonRes["key"].asUInt64();
	sqn = rand_num + 1;
	xres = key + sqn + rand_num;
	autn_num = xres + 1;
	ck = xres + 2;
	ik = xres + 3;
	k_asme = ck + ik + sqn + plmn_id;

	jsonRes.clear();
	jsonRes["autn_num"] = touint64(autn_num);
	jsonRes["rand_num"] = touint64(rand_num);
	jsonRes["xres"] = touint64(xres);
	jsonRes["k_asme"] = touint64(k_asme);

	std::cout<<"requestPkt.autn_num is (ausf.cpp) "<< autn_num << endl;

	Json::FastWriter fastWriter;
	return fastWriter.write(jsonRes);
}

std::string Ausf::handle_location_update(Json::Value &jsonPkt, int worker_id) {
	uint64_t imsi;
	uint64_t default_apn;
	uint32_t mmei;

	default_apn = 1;
	imsi = jsonPkt["imsi"].asUInt64();
	mmei = jsonPkt["mmei"].asUInt();

	TRACE(cout<<"Sending out the packet to the udm"<<endl;)

	Json::Value reqPkt, jsonRes;
	reqPkt["imsi"] = touint64(imsi);
	reqPkt["mmei"] = touint(mmei);

	// Ignore return value when we dont expect any return in jsonRes.
	send_and_receive(
		g_udm_ip_addr, 
		UDM_PORT_START_RANGE + worker_id, 
		"/Nudm_UECM/SetLOCInfo",
		reqPkt, jsonRes
	);

	TRACE(cout<<"Packet sent to udm"<<endl;)
	TRACE(cout << "ausf_handleautoinforeq:" << " loc updated" << endl;)

	jsonRes.clear();
	jsonRes["default_apn"] = touint64(default_apn);
	TRACE(cout << "ausf_handleautoinforeq:" << " loc update complete sent to amf" << endl;)

	Json::FastWriter fastWriter;
	return fastWriter.write(jsonRes);
}

Ausf::~Ausf() {

}
