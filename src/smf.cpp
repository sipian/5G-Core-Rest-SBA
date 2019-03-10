#include "smf.h"
#include "discover.h"
#include "ports.h"
#include "rest_utils.h"
#include <jsoncpp/json/json.h>

// string g_trafmon_ip_addr = "172.26.0.2";
// string g_upf_smf_ip_addr = "172.26.0.7";
// string upf_s1_ip_addr = "172.26.0.7";
// string smf_upf_ip_addr = "172.26.0.6";//smf-upf
// string smf_amf_ip_addr = "172.26.0.6";//smf-amf
// string upf_s11_ip_addr="172.26.0.7";
// string g_udm_ip_addr="172.26.0.3";

string g_trafmon_ip_addr = resolve_host("ran");
string g_upf_smf_ip_addr = resolve_host("upf");
string upf_s1_ip_addr = resolve_host("upf");
string smf_upf_ip_addr = resolve_host("smf");
string smf_amf_ip_addr = resolve_host("smf");
string upf_s11_ip_addr = resolve_host("upf");
string g_udm_ip_addr = resolve_host("udm");

int g_trafmon_port = 4000;
int g_upf_smf_port = 7000;
int upf_s1_port = 7100;
int upf_s11_port=7300;
int smf_upf_port = 7200;//smf-upf
int smf_amf_port = 8500;//smf-amf
int g_udm_port = 6001;

uint64_t g_timer = 100;
uint64_t guti=0;


UeContext::UeContext() {
	emm_state = 0;
	ecm_state = 0;
	imsi = 0;
	string ip_addr = "";
	enodeb_s1ap_ue_id = 0;
	mme_s1ap_ue_id = 0;
	tai = 0;
	tau_timer = 0;
	ksi_asme = 0; 
	k_nas_enc = 0; 
	k_nas_int = 0; 
	nas_enc_algo = 0; 
	nas_int_algo = 0; 
	count = 1;
	bearer = 0;
	dir = 1;
	default_apn = 0; 
	apn_in_use = 0; 
	eps_bearer_id = 0; 
	e_rab_id = 0;
	s1_uteid_ul = 0; 
	s1_uteid_dl = 0; 
	xres = 0;
	nw_type = 0;
	nw_capability = 0;	
	upf_smf_ip_addr = "";	
	upf_smf_port = 0;
	s11_cteid_mme = 0;
	s11_cteid_sgw = 0;	
}

void UeContext::init(uint64_t arg_imsi, uint32_t arg_enodeb_s1ap_ue_id, uint32_t arg_mme_s1ap_ue_id, uint64_t arg_tai, uint16_t arg_nw_capability) {
	imsi = arg_imsi;
	enodeb_s1ap_ue_id = arg_enodeb_s1ap_ue_id;
	mme_s1ap_ue_id = arg_mme_s1ap_ue_id;
	tai = arg_tai;
	nw_capability = arg_nw_capability;
}

UeContext::~UeContext()
{
}

Smf::Smf() {
	clrstl();
	g_sync.mux_init(uectx_mux);
}

void Smf::clrstl() {
	ue_ctx.clear();
}

void Smf::handle_create_session(CreateSMContextRequestPacket &requestPkt, CreateSMContextResponsPacket &responsePkt, UdpClient &upf_client, int worker_id) {
	vector<uint64_t> tai_list;
	uint64_t guti;
	uint64_t imsi;
	uint64_t apn_in_use;
	uint64_t tai;
	uint64_t k_enodeb;
	uint64_t k_nas_enc;
	uint64_t k_nas_int;
	uint64_t tau_timer;
	uint32_t s11_cteid_mme;
	uint32_t s11_cteid_sgw;
	uint32_t s1_uteid_ul;
	uint16_t nw_capability;
	uint8_t eps_bearer_id;
	uint8_t e_rab_id;
	string ue_ip_addr;
	int tai_list_size;
	bool res;
	
	TRACE(cout << "after receiving from amf "<< endl;)
	guti = requestPkt.guti;
	TRACE(cout << "guti:" << guti << endl;)
	if (guti == 0) {
		TRACE(cout << "smf_handlecreatesession: zero guti : " << guti << endl;)
		g_utils.handle_type1_error(-1, "Zero guti: smf_handlecreatesession");
	}	
	
	imsi = requestPkt.imsi;
	s11_cteid_mme = requestPkt.s11_cteid_mme;
	eps_bearer_id = requestPkt.eps_bearer_id;
	apn_in_use = requestPkt.apn_in_use;
	tai = requestPkt.tai;


	// g_sync.mlock(uectx_mux);
	// ue_ctx[guti].s11_cteid_mme = s11_cteid_mme;
	// ue_ctx[guti].eps_bearer_id = eps_bearer_id;
	// ue_ctx[guti].imsi = imsi;
	// ue_ctx[guti].apn_in_use = apn_in_use;
	// ue_ctx[guti].tai = tai;
	// g_sync.munlock(uectx_mux);
	// s11_cteid_mme = ue_ctx[guti].s11_cteid_mme;
	// eps_bearer_id = ue_ctx[guti].eps_bearer_id;
	// imsi = ue_ctx[guti].imsi;
	// apn_in_use = ue_ctx[guti].apn_in_use;
	// tai = ue_ctx[guti].tai;

	Json::Value reqPkt, jsonRes;
	reqPkt["guti"] = to_string(guti);
	reqPkt["s11_cteid_mme"] = to_string(s11_cteid_mme);
	reqPkt["eps_bearer_id"] = to_string(eps_bearer_id);
	reqPkt["imsi"] = to_string(imsi);
	reqPkt["apn_in_use"] = to_string(apn_in_use);
	reqPkt["tai"] = to_string(tai);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/18",
		reqPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
	}


	TRACE(cout<<"UE CTX request for updation sent to udm"<<endl;)

	s11_cteid_mme = jsonRes["s11_cteid_mme"].asUInt();
	eps_bearer_id = jsonRes["eps_bearer_id"].asUInt();
	imsi = jsonRes["imsi"].asUInt64();
	apn_in_use = jsonRes["apn_in_use"].asUInt64();
	tai = jsonRes["tai"].asUInt64();

	upf_client.set_server(g_upf_smf_ip_addr, g_upf_smf_port);

	Packet pkt;
	pkt.clear_pkt();
	pkt.append_item(s11_cteid_mme);
	pkt.append_item(imsi);
	pkt.append_item(eps_bearer_id);
	pkt.append_item(apn_in_use);
	pkt.append_item(tai);
	pkt.prepend_gtp_hdr(2, 1, pkt.len, 0);
	upf_client.snd(pkt);//send to upf
	TRACE(cout << "smf_createsession: create session request sent to upf: " << guti << endl;)

	upf_client.rcv(pkt);//receive response from upf
	TRACE(cout << "smf_createsession: create session response received from upf: " << guti << endl;)

	pkt.extract_gtp_hdr();
	pkt.extract_item(s11_cteid_sgw);
	pkt.extract_item(ue_ip_addr);
	pkt.extract_item(s1_uteid_ul);
	
	// g_sync.mlock(uectx_mux);
	// ue_ctx[guti].ip_addr = ue_ip_addr;
	// ue_ctx[guti].s11_cteid_sgw = s11_cteid_sgw;
	// ue_ctx[guti].s1_uteid_ul = s1_uteid_ul;
	// ue_ctx[guti].tai_list.clear();
	// ue_ctx[guti].tai_list.push_back(ue_ctx[guti].tai);
	// ue_ctx[guti].tau_timer = g_timer;
	// ue_ctx[guti].e_rab_id = ue_ctx[guti].eps_bearer_id;
	// e_rab_id = ue_ctx[guti].e_rab_id;
	// k_enodeb = ue_ctx[guti].k_enodeb;
	// tai_list = ue_ctx[guti].tai_list;
	// tau_timer = ue_ctx[guti].tau_timer;
	// g_sync.munlock(uectx_mux);

	reqPkt.clear();
	jsonRes.clear();
	reqPkt["guti"] = to_string(guti);
	reqPkt["ue_ip_addr"] = ue_ip_addr;
	reqPkt["s11_cteid_sgw"] = to_string(s11_cteid_sgw);
	reqPkt["s1_uteid_ul"] = to_string(s1_uteid_ul);
	reqPkt["g_timer"] = to_string(g_timer);

	parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/19",
		reqPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
	}

	e_rab_id = jsonRes["e_rab_id"].asUInt();
	k_enodeb = jsonRes["k_enodeb"].asUInt64();
	tau_timer = jsonRes["tau_timer"].asUInt64();	
	tai_list.clear();
	Json::Value taiListVals = jsonRes["tai_list"];
	uint64_t taiListElem;
	for( Json::ValueIterator itr = taiListVals.begin() ; itr != taiListVals.end() ; itr++ ) {
		taiListElem = itr->asUInt64();
		tai_list.push_back(taiListElem);
	}


	res = true;
	tai_list_size = 1;

	responsePkt.guti = guti;
	responsePkt.eps_bearer_id = eps_bearer_id;
	responsePkt.e_rab_id = e_rab_id;
	responsePkt.s1_uteid_ul = s1_uteid_ul;
	responsePkt.s11_cteid_sgw = s11_cteid_mme;
	responsePkt.k_enodeb = k_enodeb;
	responsePkt.tai_list_size = tai_list_size;
	responsePkt.tai_list = tai_list;
	responsePkt.tau_timer = tau_timer;
	responsePkt.ue_ip_addr = ue_ip_addr;
	responsePkt.upf_s1_ip_addr = upf_s1_ip_addr;
	responsePkt.upf_s1_port = upf_s1_port;
	responsePkt.res = res;
}

void Smf::handle_modify_bearer(UpdateSMContextRequestPacket &requestPkt, UpdateSMContextResponsePacket &responsePkt, UdpClient &upf_client, int worker_id) {
	uint64_t guti;
	uint32_t s1_uteid_dl;
	uint32_t s11_cteid_sgw;
	uint8_t eps_bearer_id;
	bool res;
	string enodeb_ip_addr;
	int enodeb_port;

	guti = requestPkt.guti;
	if (guti == 0) {
		TRACE(cout << "smf_handlemodifybearer: zero guti : " << guti << endl;)
		g_utils.handle_type1_error(-1, "Zero guti: smf_handlemodifybearer");
	}
	
	eps_bearer_id = requestPkt.eps_bearer_id;
	s1_uteid_dl = requestPkt.s1_uteid_dl;
	g_trafmon_ip_addr = requestPkt.g_trafmon_ip_addr;;
	g_trafmon_port = requestPkt.g_trafmon_port;

	upf_client.set_server(g_upf_smf_ip_addr, g_upf_smf_port);

	Json::Value reqPkt, jsonRes;
	reqPkt["guti"] = to_string(guti);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/20",
		reqPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
	}

	eps_bearer_id = jsonRes["eps_bearer_id"].asUInt();
	s1_uteid_dl = jsonRes["s1_uteid_dl"].asUInt();
	s11_cteid_sgw = jsonRes["s11_cteid_sgw"].asUInt();

	Packet pkt;
	pkt.clear_pkt();
	pkt.append_item(eps_bearer_id);
	pkt.append_item(s1_uteid_dl);
	pkt.append_item(g_trafmon_ip_addr);
	pkt.append_item(g_trafmon_port);
	pkt.prepend_gtp_hdr(2, 2, pkt.len, s11_cteid_sgw);
	upf_client.snd(pkt);
	TRACE(cout << "smf_handlemodifybearer: modify bearer request sent to upf: " << guti << endl;)

	upf_client.rcv(pkt);
	TRACE(cout << "smf_handlemodifybearer: modify bearer response received from upf: " << guti << endl;)

	pkt.extract_gtp_hdr();
	pkt.extract_item(res);
	if (res == false) {
		TRACE(cout << "smf_handlemodifybearer: modify bearer failure: " << guti << endl;)
	}
	responsePkt.res = res;
}

void Smf::handle_detach(ReleaseSMContextRequestPacket &requestPkt, ReleaseSMContextResponsePacket &responsePkt, UdpClient &upf_client, int worker_id) {
	uint64_t guti;
	uint64_t k_nas_enc;
	uint64_t k_nas_int;
	uint64_t ksi_asme;
	uint64_t detach_type;
	uint64_t tai;
	uint32_t s11_cteid_sgw;
	uint8_t eps_bearer_id;
	bool res;
	
	guti = requestPkt.guti;
	if (guti == 0) {
		TRACE(cout << "smf_handledetach: zero guti  : " << guti << endl;)
		g_utils.handle_type1_error(-1, "Zero guti: smf_handledetach");
	}
	eps_bearer_id = requestPkt.eps_bearer_id;
	tai = requestPkt.tai;

	upf_client.set_server(g_upf_smf_ip_addr, g_upf_smf_port);
	//
	// g_sync.mlock(uectx_mux);
	// s11_cteid_sgw = ue_ctx[guti].s11_cteid_sgw;
	// g_sync.munlock(uectx_mux);
	//

	Json::Value reqPkt, jsonRes;
	reqPkt["guti"] = to_string(guti);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/21",
		reqPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
	}

	s11_cteid_sgw = jsonRes["s11_cteid_sgw"].asUInt();

	TRACE(cout << "smf_handledetach: detach req received from amf: " << guti << endl;)
	
	Packet pkt;
	pkt.clear_pkt();
	pkt.append_item(eps_bearer_id);
	pkt.append_item(tai);
	pkt.prepend_gtp_hdr(2, 3, pkt.len, s11_cteid_sgw);
	upf_client.snd(pkt);
	TRACE(cout << "smf_handledetach: detach request sent to upf: " << guti << endl;)

	upf_client.rcv(pkt);
	TRACE(cout << "smf_handledetach: detach response received from upf: " << guti << endl;)

	pkt.extract_gtp_hdr();
	pkt.extract_item(res);
	if (res == false) {
		TRACE(cout << "smf_handledetach: upf detach failure: " << guti << endl;)
		return;		
	}

	TRACE(cout << "smf_handledetach: ue entry removed: " << guti << endl;)
	TRACE(cout << "smf_handledetach: detach successful: " << guti << endl;)

	responsePkt.res = res;
}


uint32_t Smf::get_s11cteidmme(uint64_t guti) {
	uint32_t s11_cteid_mme;
	string tem;

	tem = to_string(guti);
	tem = tem.substr(7, -1); /* Extracting only the last 9 digits of UE MSISDN */
	s11_cteid_mme = stoull(tem);
	return s11_cteid_mme;
}

Smf::~Smf()
{
}
