#include "ausf.h"
#include "discover.h"
#include "ports.h"
#include "rest_utils.h"

// string g_ausf_ip_addr = "192.168.130.162";
string g_ausf_ip_addr = resolve_host("ausf");
int g_ausf_port = 6000;

// string g_udm_ip_addr = "192.168.130.120";
string g_udm_ip_addr = resolve_host("udm");
int g_udm_port = 6001;

Ausf::Ausf() {
	g_sync.mux_init(mysql_client_mux);
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
	reqPkt["imsi"] = to_string(imsi);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr, 
		UDM_PORT_START_RANGE + worker_id, 
		"/Nudm_UECM/1",
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
	jsonRes["autn_num"] = to_string(autn_num);
	jsonRes["rand_num"] = to_string(rand_num);
	jsonRes["xres"] = to_string(xres);
	jsonRes["k_asme"] = to_string(k_asme);

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
	reqPkt["imsi"] = to_string(imsi);
	reqPkt["mmei"] = to_string(mmei);

	// Ignore return value when we dont expect any return in jsonRes.
	send_and_receive(
		g_udm_ip_addr, 
		UDM_PORT_START_RANGE + worker_id, 
		"/Nudm_UECM/2",
		reqPkt, jsonRes
	);

	TRACE(cout<<"Packet sent to udm"<<endl;)
	TRACE(cout << "ausf_handleautoinforeq:" << " loc updated" << endl;)

	jsonRes.clear();
	jsonRes["default_apn"] = to_string(default_apn);
	TRACE(cout << "ausf_handleautoinforeq:" << " loc update complete sent to amf" << endl;)

	Json::FastWriter fastWriter;
	return fastWriter.write(jsonRes);
}

Ausf::~Ausf() {

}
