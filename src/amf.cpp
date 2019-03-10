#include "amf.h"
#include "discover.h"
#include "rest_utils.h"
#include <jsoncpp/json/json.h>
#include <nghttp2/asio_http2_client.h>

using boost::asio::ip::tcp;

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::client;
using namespace std;
using namespace std::chrono;

// string g_trafmon_ip_addr = "172.26.0.2";
// string g_amf_ip_addr = "172.26.0.5";
// string g_ausf_ip_addr = "172.26.0.4";
// string g_upf_smf_ip_addr = "172.26.0.7";
// string smf_amf_ip_addr = "172.26.0.6";
// string g_upf_s1_ip_addr="172.26.0.7";
// string g_upf_s11_ip_addr="172.26.0.7";
// string g_udm_ip_addr="172.26.0.3";

string g_trafmon_ip_addr = resolve_host("ran");
string g_amf_ip_addr = resolve_host("amf");
string g_ausf_ip_addr = resolve_host("ausf");
string g_upf_smf_ip_addr = resolve_host("upf");
string smf_amf_ip_addr = resolve_host("smf");
string g_upf_s1_ip_addr = resolve_host("upf");
string g_upf_s11_ip_addr = resolve_host("upf");
string g_udm_ip_addr = resolve_host("udm");

int g_trafmon_port = 4000;
int g_amf_port = 5000;
int g_ausf_port = 6000;
int smf_amf_port = 8500;
int g_upf_smf_port = 8000;
int g_upf_s1_port=7100;
int g_upf_s11_port=7300;
uint64_t g_timer = 100;
string t_ran_ip_addr = "10.129.26.169";
int t_ran_port = 4905;
string s_ran_ip_addr = "10.129.26.169";
int s_ran_port = 4905;
int g_udm_port = 6001;


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
	k_asme = 0; 
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
	s11_cteid_amf = 0;
	s11_cteid_upf = 0;	
}

void UeContext::init(uint64_t arg_imsi, uint32_t arg_enodeb_s1ap_ue_id, uint32_t arg_mme_s1ap_ue_id, uint64_t arg_tai, uint16_t arg_nw_capability) {
	imsi = arg_imsi;
	enodeb_s1ap_ue_id = arg_enodeb_s1ap_ue_id;
	mme_s1ap_ue_id = arg_mme_s1ap_ue_id;
	tai = arg_tai;
	nw_capability = arg_nw_capability;
}

UeContext::~UeContext() {

}

AmfIds::AmfIds() {
	mcc = 1;
	mnc = 1;
	plmn_id = g_telecom.get_plmn_id(mcc, mnc);
	amfgi = 1;
	amfc = 1;
	amfi = g_telecom.get_mmei(amfgi, amfc);
	guamfi = g_telecom.get_gummei(plmn_id, amfi);
}

AmfIds::~AmfIds() {
	
}

Amf::Amf() {
	ue_count = 0;
	clrstl();
	g_sync.mux_init(s1amfid_mux);
	g_sync.mux_init(uectx_mux);
}

void Amf::clrstl() {
	s1amf_id.clear();
	ue_ctx.clear();
}

uint32_t Amf::get_s11cteidamf(uint64_t guti) {
	uint32_t s11_cteid_amf;
	string tem;

	tem = to_string(guti);
	tem = tem.substr(7, -1); /* Extracting only the last 9 digits of UE MSISDN */
	s11_cteid_amf = stoull(tem);
	return s11_cteid_amf;
}

void Amf::handle_initial_attach(int conn_fd, Packet pkt, int worker_id) {

	uint64_t imsi;
	uint64_t tai;
	uint64_t ksi_asme;
	uint16_t nw_type;
	uint16_t nw_capability;
	uint64_t autn_num;
	uint64_t rand_num;
	uint64_t xres;
	uint64_t k_asme;
	uint32_t enodeb_s1ap_ue_id;
	uint32_t mme_s1ap_ue_id;
	uint64_t guti;
	uint64_t num_autn_vectors;

	boost::system::error_code ec;
	boost::asio::io_service io_service;

	num_autn_vectors = 1;
	pkt.extract_item(imsi);
	pkt.extract_item(tai);
	pkt.extract_item(ksi_asme); /* No use in this case */
	pkt.extract_item(nw_capability); /* No use in this case */

	enodeb_s1ap_ue_id = pkt.s1ap_hdr.enodeb_s1ap_ue_id;
	guti = g_telecom.get_guti(amf_ids.guamfi, imsi);

	TRACE(cout << "amf_handleinitialattach:" << " initial attach req received: " << guti << endl;)

	g_sync.mlock(s1amfid_mux);
	ue_count++;
	mme_s1ap_ue_id = ue_count;
	s1amf_id[mme_s1ap_ue_id] = guti;
	g_sync.munlock(s1amfid_mux);

	// g_sync.mlock(uectx_mux);
	// ue_ctx[guti].init(imsi, enodeb_s1ap_ue_id, mme_s1ap_ue_id, tai, nw_capability);
	// g_sync.munlock(uectx_mux);


	Json::Value requestPkt, jsonRes;
	requestPkt["guti"] = to_string(guti);
	requestPkt["imsi"] = to_string(imsi);
	requestPkt["enodeb_s1ap_ue_id"] = to_string(enodeb_s1ap_ue_id);
	requestPkt["mme_s1ap_ue_id"] = to_string(mme_s1ap_ue_id);
	requestPkt["tai"] = to_string(tai);
	requestPkt["nw_capability"] = to_string(nw_capability);

	send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/3",
		requestPkt, jsonRes
	);

	nw_type = ue_ctx[guti].nw_type;
	TRACE(cout << "amf_handleinitialattach:" << ":ue entry added: " << guti << endl;)
	
	requestPkt.clear();
	jsonRes.clear();
	requestPkt["imsi"] = to_string(imsi);
	requestPkt["plmn_id"] = to_string(amf_ids.plmn_id);
	requestPkt["num_autn_vectors"] = to_string(num_autn_vectors);
	requestPkt["nw_type"] = to_string(nw_type);
	bool parsingSuccessful = send_and_receive(
		g_ausf_ip_addr, 
		AUSF_AMF_PORT_START_RANGE + worker_id, 
		"/Nausf_UEAuthentication",
		requestPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
		cout << "amf_handleinitialattach: Response received from AUSF parsing failed" << endl;
	}

	TRACE(cout << "amf_handleinitialattach :: Received from : " << jsonRes.asString() << endl;)
	TRACE(cout << "amf_handleinitialattach :: " << " request sent to ausf: " << guti << endl;)
	
	// TODO: have isMember check?
	autn_num = jsonRes["autn_num"].asUInt64();
	rand_num = jsonRes["rand_num"].asUInt64();
	xres = jsonRes["xres"].asUInt64();
	k_asme = jsonRes["k_asme"].asUInt64();

	// g_sync.mlock(uectx_mux);
	// ue_ctx[guti].xres = xres;
	// ue_ctx[guti].k_asme = k_asme;
	// ksi_asme = 1;
	// ue_ctx[guti].ksi_asme = ksi_asme;
	// g_sync.munlock(uectx_mux);
	//
	// will send the UPDATE request to the udm for updation of ue_context.

	requestPkt.clear();
	jsonRes.clear();
	requestPkt["guti"] = to_string(guti);
	requestPkt["xres"] = to_string(xres);
	requestPkt["k_asme"] = to_string(k_asme);
	requestPkt["ksi_asme"] = "1";
	send_and_receive(
		g_ausf_ip_addr, 
		AUSF_AMF_PORT_START_RANGE + worker_id, 
		"/Nudm_UECM/4",
		requestPkt, jsonRes
	);


	// TRACE(cout << "amf_handleinitialattach:" << " autn:" << autn_num <<" rand:" << rand_num << " xres:" << xres << " k_asme:" << k_asme << " " << guti << endl;)

	pkt.clear_pkt();
	pkt.append_item(autn_num);
	pkt.append_item(rand_num);
	pkt.append_item(ksi_asme);
	pkt.prepend_s1ap_hdr(1, pkt.len, enodeb_s1ap_ue_id, mme_s1ap_ue_id);
	server.snd(conn_fd, pkt);
	TRACE(cout << "amf_handleinitialattach:" << " autn request sent to ran: " << guti << endl;	)
}

bool Amf::handle_autn(int conn_fd, Packet pkt, int worker_id) {

	uint64_t guti;
	uint64_t res;
	uint64_t xres;

	guti = get_guti(pkt);
	if (guti == 0) {
		TRACE(cout << "amf_handleautn:" << " zero guti " << pkt.s1ap_hdr.mme_s1ap_ue_id << " " << pkt.len << ": " << guti << endl;)
		g_utils.handle_type1_error(-1, "Zero guti: amf_handleautn");
	}
	pkt.extract_item(res);
	// g_sync.mlock(uectx_mux);
	// xres = ue_ctx[guti].xres;
	// g_sync.munlock(uectx_mux);

	pkt.clear_pkt();

	Json::Value reqPkt, jsonRes;
	reqPkt["guti"] = to_string(guti);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/5",
		reqPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
		cout << "amf_handleautn: Response received from UDM parsing failed" << endl;
	}

	TRACE(cout<<"amf_handleautn: Packet sent to the udm for handle autn ue ctx request"<<endl;)

	xres = jsonRes["xres"].asUInt64();

	if (res == xres) {
		/* Success */
		TRACE(cout << "amf_handleautn:" << " Authentication successful: " << guti << endl;)
		return true;
	}
	else {
		rem_itfid(pkt.s1ap_hdr.mme_s1ap_ue_id);
		rem_uectx(guti);				
		return false;
	}
}

void Amf::handle_security_mode_cmd(int conn_fd, Packet pkt, int worker_id) {
	uint64_t guti;
	uint64_t ksi_asme;
	uint16_t nw_capability;
	uint64_t nas_enc_algo;
	uint64_t nas_int_algo;
	uint64_t k_nas_enc;
	uint64_t k_nas_int;

	guti = get_guti(pkt);
	if (guti == 0) {
		TRACE(cout << "amf_handlesecuritymodecmd:" << " zero guti " << pkt.s1ap_hdr.mme_s1ap_ue_id << " " << pkt.len << ": " << guti << endl;		)
		g_utils.handle_type1_error(-1, "Zero guti: amf_handlesecuritymodecmd");
	}	
	set_crypt_context(guti, worker_id);
	set_integrity_context(guti, worker_id);

	// will rather send the request for ue_ctx to the amf

	// g_sync.mlock(uectx_mux);
	// ksi_asme = ue_ctx[guti].ksi_asme;
	// nw_capability = ue_ctx[guti].nw_capability;
	// nas_enc_algo = ue_ctx[guti].nas_enc_algo;
	// nas_int_algo = ue_ctx[guti].nas_int_algo;
	// k_nas_enc = ue_ctx[guti].k_nas_enc;
	// k_nas_int = ue_ctx[guti].k_nas_int;
	// g_sync.munlock(uectx_mux);

	pkt.clear_pkt();

	Json::Value reqPkt, jsonRes;
	reqPkt["guti"] = to_string(guti);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/6",
		reqPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
		cout << "amf_handlesecuritymodecmd: Response received from UDM parsing failed" << endl;
	}

	TRACE(cout<<"amf_handlesecuritymodecmd: Packet sent to udm for the packet security mode cmd"<<endl;)

	ksi_asme = jsonRes["ksi_asme"].asUInt64();
	nw_capability = jsonRes["nw_capability"].asUInt();
	nas_enc_algo = jsonRes["nas_enc_algo"].asUInt64();
	nas_int_algo = jsonRes["nas_int_algo"].asUInt64();
	k_nas_enc = jsonRes["k_nas_enc"].asUInt64();
	k_nas_int = jsonRes["k_nas_int"].asUInt64();

	// continue with the normal execution
	pkt.clear_pkt();
	pkt.append_item(ksi_asme);
	pkt.append_item(nw_capability);
	pkt.append_item(nas_enc_algo);
	pkt.append_item(nas_int_algo);
	if (HMAC_ON) {
		g_integrity.add_hmac(pkt, k_nas_int);
	}
	pkt.prepend_s1ap_hdr(2, pkt.len, pkt.s1ap_hdr.enodeb_s1ap_ue_id, pkt.s1ap_hdr.mme_s1ap_ue_id);
	server.snd(conn_fd, pkt);
	TRACE(cout << "amf_handlesecuritymodecmd:" << " security mode command sent: " << pkt.len << ": " << guti << endl;)
}

void Amf::set_crypt_context(uint64_t guti, int worker_id) {
//
	// g_sync.mlock(uectx_mux);
	// ue_ctx[guti].nas_enc_algo = 1;
	// ue_ctx[guti].k_nas_enc = ue_ctx[guti].k_asme + ue_ctx[guti].nas_enc_algo + ue_ctx[guti].count + ue_ctx[guti].bearer + ue_ctx[guti].dir;
	// g_sync.munlock(uectx_mux);
//

	// will send out the request of UE CTX to the udm


	Json::Value reqPkt, jsonRes;
	reqPkt["guti"] = to_string(guti);

	send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/7",
		reqPkt, jsonRes
	);

	TRACE(cout<<"amf_setcryptcontext: Packet sent for ue ctx to udm set encrypt context"<<endl;)
}

void Amf::set_integrity_context(uint64_t guti, int worker_id) {
	Json::Value reqPkt, jsonRes;
	reqPkt["guti"] = to_string(guti);

	send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/8",
		reqPkt, jsonRes
	);

	TRACE(cout<<"amf_setintegritycontext: packet sent to udm for update set integrity context"<<endl;)
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].nas_int_algo = 1;
	ue_ctx[guti].k_nas_int = ue_ctx[guti].k_asme + ue_ctx[guti].nas_int_algo + ue_ctx[guti].count + ue_ctx[guti].bearer + ue_ctx[guti].dir;
	g_sync.munlock(uectx_mux);
}

bool Amf::handle_security_mode_complete(int conn_fd, Packet pkt, int worker_id) {
	uint64_t guti;
	uint64_t k_nas_enc;
	uint64_t k_nas_int;
	bool res;

	guti = get_guti(pkt);
	if (guti == 0) {
		TRACE(cout << "amf_handlesecuritymodecomplete:" << " zero guti " << pkt.s1ap_hdr.mme_s1ap_ue_id << " " << pkt.len << ": " << guti << endl;		)
		g_utils.handle_type1_error(-1, "Zero guti: amf_handlesecuritymodecomplete");
	}

	Json::Value reqPkt, jsonRes;
	reqPkt["guti"] = to_string(guti);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/9",
		reqPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
		cout << "amf_handlesecuritymodecomplete: Response received from UDM parsing failed" << endl;
	}

	TRACE(cout<<"amf_handlesecuritymodecomplete: Packet sent to udm for the packet security mode complete"<<endl;)
	// g_sync.mlock(uectx_mux);
	// k_nas_enc = ue_ctx[guti].k_nas_enc;
	// k_nas_int = ue_ctx[guti].k_nas_int;
	// g_sync.munlock(uectx_mux);
	k_nas_enc = jsonRes["k_nas_enc"].asUInt64();
	k_nas_int = jsonRes["k_nas_int"].asUInt64();

	TRACE(cout << "amf_handlesecuritymodecomplete:" << " security mode complete received: " << pkt.len << ": " << guti << endl;)

	if (HMAC_ON) {
		res = g_integrity.hmac_check(pkt, k_nas_int);
		if (!res) {
			TRACE(cout << "amf_handlesecuritymodecomplete:" << " hmac failure: " << guti << endl;)
			g_utils.handle_type1_error(-1, "hmac failure: amf_handlesecuritymodecomplete");
		}		
	}
	if (ENC_ON) {
		g_crypt.dec(pkt, k_nas_enc);
	}
	pkt.extract_item(res);
	if (!res) {
		TRACE(cout << "amf_handlesecuritymodecomplete:" << " security mode complete failure: " << guti << endl;)
		return false;
	}
	else {
		TRACE(cout << "amf_handlesecuritymodecomplete:" << " security mode complete success: " << guti << endl;)
		return true;
	}
}

void Amf::handle_location_update(Packet pkt, int worker_id) {
	uint64_t guti;
	uint64_t imsi;
	uint64_t default_apn;

	guti = get_guti(pkt);
	if (guti == 0) {
		TRACE(cout << "amf_handlelocationupdate:" << " zero guti " << pkt.s1ap_hdr.mme_s1ap_ue_id << " " << pkt.len << ": " << guti << endl;		)
		g_utils.handle_type1_error(-1, "Zero guti: amf_handlelocationupdate");
	}		
	// g_sync.mlock(uectx_mux);
	// imsi = ue_ctx[guti].imsi;
	// g_sync.munlock(uectx_mux);
	Json::Value reqPkt, jsonRes;
	reqPkt["guti"] = to_string(guti);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/10",
		reqPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
		cout << "amf_handlelocationupdate: Response received from UDM parsing failed" << endl;
	}

	TRACE(cout<<"amf_handlelocationupdate: Packet sent to udm for the location update"<<endl;)

	imsi = jsonRes["imsi"].asUInt64();
	
	reqPkt.clear();
	jsonRes.clear();
	reqPkt["imsi"] = to_string(imsi);
	reqPkt["mmei"] = to_string(amf_ids.amfi);

	parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		AUSF_AMF_PORT_START_RANGE + worker_id,
		"/Nausf_UELocationUpdate",
		reqPkt, jsonRes
	);
	TRACE(cout << "amf_handlelocationupdate:" << " loc update response received from ausf: " << guti << endl;)

	default_apn = jsonRes["default_apn"].asUInt64();
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].default_apn = default_apn;
	ue_ctx[guti].apn_in_use = ue_ctx[guti].default_apn;
	g_sync.munlock(uectx_mux);
}

void Amf::handle_create_session(int conn_fd, Packet pkt, int worker_id) {

	vector<uint64_t> tai_list;
	uint64_t guti;
	uint64_t imsi;
	uint64_t apn_in_use;
	uint64_t tai;
	uint64_t k_enodeb;
	uint64_t k_nas_enc;
	uint64_t k_nas_int;
	uint64_t tau_timer;
	uint32_t s11_cteid_amf;
	uint32_t s11_cteid_upf;
	uint32_t s1_uteid_ul;
	uint16_t nw_capability;
	uint8_t eps_bearer_id;
	uint8_t e_rab_id;
	string upf_smf_ip_addr;
	int upf_smf_port;
	string ue_ip_addr;
	int tai_list_size;
	bool res;

	guti = get_guti(pkt);
	if (guti == 0) {
		TRACE(cout << "amf_createsession:" << " zero guti " << pkt.s1ap_hdr.mme_s1ap_ue_id << " " << pkt.len << ": " << guti << endl;)
		g_utils.handle_type1_error(-1, "Zero guti: amf_handlecreatesession");
	}

	eps_bearer_id = 5;
	set_upf_info(guti, worker_id);

	//
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].s11_cteid_amf = get_s11cteidamf(guti);
	ue_ctx[guti].eps_bearer_id = eps_bearer_id;
	// s11_cteid_amf = ue_ctx[guti].s11_cteid_amf;
	// imsi = ue_ctx[guti].imsi;
	// eps_bearer_id = ue_ctx[guti].eps_bearer_id;
	// upf_smf_ip_addr = ue_ctx[guti].upf_smf_ip_addr;
	// upf_smf_port = ue_ctx[guti].upf_smf_port;
	// apn_in_use = ue_ctx[guti].apn_in_use;
	// tai = ue_ctx[guti].tai;
	g_sync.munlock(uectx_mux);
	//

	pkt.clear_pkt();

	Json::Value requestPkt, jsonRes;
	requestPkt["guti"] = to_string(guti);
	requestPkt["s11_cteid_amf"] = to_string(get_s11cteidamf(guti));
	requestPkt["eps_bearer_id"] = to_string(eps_bearer_id);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/11",
		requestPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
		cout << "amf_createsession: Response received from UDM parsing failed" << endl;
	}

	TRACE(cout << "amf_createsession:" << "Packet sent to UDM for creating session" << endl;)

	s11_cteid_amf = jsonRes["s11_cteid_amf"].asUInt();
	imsi = jsonRes["imsi"].asUInt64();
	eps_bearer_id = jsonRes["eps_bearer_id"].asUInt();
	upf_smf_ip_addr = jsonRes["upf_smf_ip_addr"].asString();
	upf_smf_port = jsonRes["upf_smf_port"].asInt();
	apn_in_use = jsonRes["apn_in_use"].asUInt64();
	tai = jsonRes["tai"].asUInt64();

	requestPkt.clear();
	jsonRes.clear();
	requestPkt["guti"] = to_string(guti);
	requestPkt["imsi"] = to_string(imsi);
	requestPkt["s11_cteid_mme"] = to_string(s11_cteid_amf);
	requestPkt["eps_bearer_id"] = to_string(eps_bearer_id);
	requestPkt["apn_in_use"] = to_string(apn_in_use);
	requestPkt["tai"] = to_string(tai);

	parsingSuccessful = send_and_receive(
		smf_amf_ip_addr, 
		SMF_AMF_PORT_START_RANGE + worker_id, 
		"/Nsmf_PDUSession/CreateSMContext",
		requestPkt, jsonRes
	);

	TRACE(cout << "amf_createsession :: Received from SMF : " << jsonRes.asString() << endl;)
	TRACE(cout << "amf_createsession:" << " create session response received smf: " << guti << endl;)

	//TODO handle respone errors
	if(!parsingSuccessful) {
		cout << "amf_createsession: Response received from SMF parsing failed" << endl;
	}

	if(jsonRes.isMember("guti")) {
		guti = jsonRes["guti"].asUInt64();
	}

	if(jsonRes.isMember("eps_bearer_id")) {
		eps_bearer_id = jsonRes["eps_bearer_id"].asUInt();
	}

	if(jsonRes.isMember("e_rab_id")) {
		e_rab_id = jsonRes["e_rab_id"].asUInt();
	}

	if(jsonRes.isMember("s1_uteid_ul")) {
		s1_uteid_ul = jsonRes["s1_uteid_ul"].asUInt();
	}

	if(jsonRes.isMember("s11_cteid_sgw")) {
		s11_cteid_upf = jsonRes["s11_cteid_sgw"].asUInt();
	}

	if(jsonRes.isMember("k_enodeb")) {
		k_enodeb = jsonRes["k_enodeb"].asUInt64();
	}

	if(jsonRes.isMember("tai_list_size")) {
		tai_list_size = jsonRes["tai_list_size"].asInt();
	}

	if(jsonRes.isMember("tai_list")) {
		tai_list.clear();
		Json::Value taiListVals = jsonRes["tai_list"];
		uint64_t taiListElem;
		for( Json::ValueIterator itr = taiListVals.begin() ; itr != taiListVals.end() ; itr++ ) {
			taiListElem = itr->asUInt64();
			tai_list.push_back(taiListElem);
		}
	}

	if(jsonRes.isMember("tau_timer")) {
		tau_timer = jsonRes["tau_timer"].asUInt64();
	}

	if(jsonRes.isMember("ue_ip_addr")) {
		ue_ip_addr = jsonRes["ue_ip_addr"].asString();
	}

	if(jsonRes.isMember("upf_s1_ip_addr")) {
		g_upf_s1_ip_addr = jsonRes["upf_s1_ip_addr"].asString();
	}

	if(jsonRes.isMember("upf_s1_port")) {
		g_upf_s1_port = jsonRes["upf_s1_port"].asInt();
	}

	if(jsonRes.isMember("res")) {
		res = jsonRes["res"].asBool();
	}

	TRACE(cout << "amf_createsession:" << " create session IP addr: " << g_upf_s1_ip_addr << ":" << g_upf_s1_port << endl;)

	//
	// g_sync.mlock(uectx_mux);
	// ue_ctx[guti].ip_addr = ue_ip_addr;
	// ue_ctx[guti].s11_cteid_upf = s11_cteid_upf;
	// ue_ctx[guti].s1_uteid_ul = s1_uteid_ul;
	// ue_ctx[guti].tai_list.clear();
	// ue_ctx[guti].tai_list.push_back(ue_ctx[guti].tai);
	// ue_ctx[guti].tau_timer = g_timer;
	// ue_ctx[guti].e_rab_id = ue_ctx[guti].eps_bearer_id;
	// ue_ctx[guti].k_enodeb = ue_ctx[guti].k_asme;
	// g_sync.munlock(uectx_mux);
	// e_rab_id = ue_ctx[guti].e_rab_id;
	// k_enodeb = ue_ctx[guti].k_enodeb;
	// nw_capability = ue_ctx[guti].nw_capability;
	// tai_list = ue_ctx[guti].tai_list;
	// tau_timer = ue_ctx[guti].tau_timer;
	// k_nas_enc = ue_ctx[guti].k_nas_enc;
	// k_nas_int = ue_ctx[guti].k_nas_int;
	//

	pkt.clear_pkt();

	requestPkt.clear();
	jsonRes.clear();
	requestPkt["guti"] = to_string(guti);
	requestPkt["ue_ip_addr"] = ue_ip_addr;
	requestPkt["s11_cteid_upf"] = to_string(s11_cteid_upf);
	requestPkt["s1_uteid_ul"] = to_string(s1_uteid_ul);
	requestPkt["tau_timer"] = to_string(tau_timer);

	parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/12",
		requestPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
		cout << "amf_createsession: Response received from UDM parsing failed" << endl;
	}

	e_rab_id = jsonRes["e_rab_id"].asUInt();
	k_enodeb = jsonRes["k_enodeb"].asUInt64();
	nw_capability = jsonRes["nw_capability"].asUInt64();
	tai_list.clear();
	Json::Value taiListVals = jsonRes["tai_list"];
	uint64_t taiListElem;
	for( Json::ValueIterator itr = taiListVals.begin() ; itr != taiListVals.end() ; itr++ ) {
		taiListElem = itr->asUInt64();
		tai_list.push_back(taiListElem);
	}

	tau_timer = jsonRes["tau_timer"].asUInt64();
	k_nas_enc = jsonRes["k_nas_enc"].asUInt64();
	k_nas_int = jsonRes["k_nas_int"].asUInt64();

	res = true;
	tai_list_size = 1;

	pkt.clear_pkt();
	pkt.append_item(guti);
	pkt.append_item(eps_bearer_id);
	pkt.append_item(e_rab_id);
	pkt.append_item(s1_uteid_ul);
	pkt.append_item(k_enodeb);
	pkt.append_item(nw_capability);
	pkt.append_item(tai_list_size);
	pkt.append_item(tai_list);
	pkt.append_item(tau_timer);
	pkt.append_item(ue_ip_addr);
	pkt.append_item(g_upf_s1_ip_addr);
	pkt.append_item(g_upf_s1_port);
	pkt.append_item(res);

	if (ENC_ON) {
		g_crypt.enc(pkt, k_nas_enc);
	}
	if (HMAC_ON) {
		g_integrity.add_hmac(pkt, k_nas_int);
	}
	pkt.prepend_s1ap_hdr(3, pkt.len, pkt.s1ap_hdr.enodeb_s1ap_ue_id, pkt.s1ap_hdr.mme_s1ap_ue_id);
	server.snd(conn_fd, pkt);
	TRACE(cout << "amf_createsession:" << " attach accept sent to ue: " << pkt.len << ": " << guti << endl;)
}

void Amf::handle_attach_complete(Packet pkt, int worker_id) {
	uint64_t guti;
	uint64_t k_nas_enc;
	uint64_t k_nas_int;
	uint32_t s1_uteid_dl;
	uint8_t eps_bearer_id;
	bool res;

	guti = get_guti(pkt);
	if (guti == 0) {
		TRACE(cout << "amf_handleattachcomplete:" << " zero guti " << pkt.s1ap_hdr.mme_s1ap_ue_id << " " << pkt.len << ": " << guti << endl;		)
		g_utils.handle_type1_error(-1, "Zero guti: amf_handleattachcomplete");
	}
	//	
	// g_sync.mlock(uectx_mux);
	// k_nas_enc = ue_ctx[guti].k_nas_enc;
	// k_nas_int = ue_ctx[guti].k_nas_int;
	// g_sync.munlock(uectx_mux);
	//

	Json::Value requestPkt, jsonRes;
	requestPkt["guti"] = to_string(guti);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/13",
		requestPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
		cout << "amf_handleattachcomplete: Response received from UDM parsing failed" << endl;
	}

	k_nas_enc = jsonRes["k_nas_enc"].asUInt64();
	k_nas_int = jsonRes["k_nas_int"].asUInt64();

	TRACE(cout << "amf_handleattachcomplete:" << " attach complete received: " << pkt.len << ": " << guti << endl;)

	if (HMAC_ON) {
		res = g_integrity.hmac_check(pkt, k_nas_int);
		if (!res) {
			TRACE(cout << "amf_handleattachcomplete:" << " hmac failure: " << guti << endl;)
			g_utils.handle_type1_error(-1, "hmac failure: amf_handleattachcomplete");
		}
	}
	if (ENC_ON) {
		g_crypt.dec(pkt, k_nas_enc);
	}
	pkt.extract_item(eps_bearer_id);
	pkt.extract_item(s1_uteid_dl);
	//
	// g_sync.mlock(uectx_mux);
	// ue_ctx[guti].s1_uteid_dl = s1_uteid_dl;
	// ue_ctx[guti].emm_state = 1;
	// g_sync.munlock(uectx_mux);
	//

	requestPkt.clear();
	jsonRes.clear();
	requestPkt["guti"] = to_string(guti);
	requestPkt["s1_uteid_dl"] = to_string(s1_uteid_dl);

	send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/14",
		requestPkt, jsonRes
	);

	TRACE(cout << "amf_handleattachcomplete:" << " attach completed: " << guti << endl;)
}

void Amf::handle_modify_bearer(Packet pkt, int worker_id) {

	uint64_t guti;
	uint32_t s1_uteid_dl;
	uint32_t s11_cteid_upf;
	uint8_t eps_bearer_id;
	bool res;

	guti = get_guti(pkt);
	if (guti == 0) {
		TRACE(cout << "amf_handlemodifybearer:" << " zero guti " << pkt.s1ap_hdr.mme_s1ap_ue_id << " " << pkt.len << ": " << guti << endl;)
		g_utils.handle_type1_error(-1, "Zero guti: amf_handlemodifybearer");
	}

	//
	// g_sync.mlock(uectx_mux);
	// eps_bearer_id = ue_ctx[guti].eps_bearer_id;
	// s1_uteid_dl = ue_ctx[guti].s1_uteid_dl;
	// s11_cteid_upf = ue_ctx[guti].s11_cteid_upf;
	// g_sync.munlock(uectx_mux);
	//


	Json::Value requestPkt, jsonRes;
	requestPkt["guti"] = to_string(guti);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/15",
		requestPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
		cout << "amf_handlemodifybearer: Response received from UDM parsing failed" << endl;
	}

	eps_bearer_id = jsonRes["eps_bearer_id"].asUInt();
	s1_uteid_dl = jsonRes["s1_uteid_dl"].asUInt();
	s11_cteid_upf = jsonRes["s11_cteid_upf"].asUInt();

	TRACE(cout << "amf_handlemodifybearer:" << " modify bearer request sent to smf: " << guti << endl;)

	requestPkt.clear();
	jsonRes.clear();
	requestPkt["guti"] = to_string(guti);
	requestPkt["s1_uteid_dl"] = to_string(s1_uteid_dl);
	requestPkt["eps_bearer_id"] = to_string(eps_bearer_id);
	requestPkt["g_trafmon_ip_addr"] = g_trafmon_ip_addr;
	requestPkt["g_trafmon_port"] = to_string(g_trafmon_port);

	parsingSuccessful = send_and_receive(
		smf_amf_ip_addr, 
		SMF_AMF_PORT_START_RANGE + worker_id, 
		"/Nsmf_PDUSession/UpdateSMContext",
		requestPkt, jsonRes
	);

	TRACE(cout << "amf_handlemodifybearer :: Received from SMF : " << jsonRes.asString() << endl;)
	TRACE(cout << "amf_handlemodifybearer:" << " modify bearer response received from smf: " << guti << endl;)


	// TODO handle response errors
	if(!parsingSuccessful) {
		TRACE(cout << "ERROR :: amf_handlemodifybearer:" << " modify bearer failure: JSON parsing failed" << endl;)
	}

	res = false; 		// Default res failed
	if(jsonRes.isMember("res")) {
		res = jsonRes["res"].asBool();
	}

	if (!res) {
		TRACE(cout << "ERROR :: amf_handlemodifybearer:" << " modify bearer failure: " << guti << endl;)
	}
	else {
		//
		g_sync.mlock(uectx_mux);
		ue_ctx[guti].ecm_state = 1;
		g_sync.munlock(uectx_mux);
		//
		TRACE(cout<<"amf_handlemodifybearer:" << " eps session setup success: " << guti << endl;)
	}
}

/* handover changes start */
void Amf::handle_handover(Packet pkt) {
	request_target_RAN(pkt);
}
void Amf::setup_indirect_tunnel(Packet pkt) {

	cout<<"set-up indirect tunnel at amf \n";

	UdpClient upf_client; //
	uint64_t guti; //
	uint32_t s1_uteid_dl_ho; //ran 2 has sent its id, dl id for upf to send data
	uint32_t s1_uteid_ul;
	uint32_t s11_cteid_upf;
	bool res;

	pkt.extract_item(s1_uteid_dl_ho);

	upf_client.conn(g_amf_ip_addr, g_upf_s11_ip_addr, g_upf_s11_port);
	guti = get_guti(pkt);
	g_sync.mlock(uectx_mux);

	s11_cteid_upf = ue_ctx[guti].s11_cteid_upf;
	g_sync.munlock(uectx_mux);
	pkt.clear_pkt();
	pkt.append_item(s1_uteid_dl_ho);
	pkt.prepend_gtp_hdr(2, 4, pkt.len, s11_cteid_upf);
	upf_client.snd(pkt);
	upf_client.rcv(pkt);

	//new indirect tunnel was setup at upf and sent to the senb for use
	pkt.extract_gtp_hdr();
	pkt.extract_item(res);//this is the indirect uplink teid for senb
	pkt.extract_item(s1_uteid_ul);

	SctpClient to_source_ran_client;
	to_source_ran_client.conn(s_ran_ip_addr, s_ran_port);

	if (res) {

		pkt.clear_pkt();
		pkt.append_item(s1_uteid_ul);

		//for msg for source ran with 8 as uid
		pkt.prepend_s1ap_hdr(8, pkt.len, pkt.s1ap_hdr.enodeb_s1ap_ue_id, pkt.s1ap_hdr.mme_s1ap_ue_id);

		to_source_ran_client.snd(pkt);//send to source RAN

	}
	cout << "indirect tunnel setup complete at amf:"<< endl;

}
void Amf::request_target_RAN( Packet pkt) {

	int handover_type;
	uint16_t s_enb;
	uint16_t t_enb;
	uint64_t guti;

	uint32_t enodeb_s1ap_ue_id; /* eNodeB S1AP UE ID */
	uint32_t mme_s1ap_ue_id;

	pkt.extract_item(handover_type);
	pkt.extract_item(s_enb);
	pkt.extract_item(t_enb);
	pkt.extract_item(enodeb_s1ap_ue_id);
	pkt.extract_item(mme_s1ap_ue_id);
	guti = get_guti(pkt);


	//send s1ap headers to target enb for use
	cout<<"req_tar_ran"<<endl;

	pkt.clear_pkt();

	pkt.append_item(t_enb);
	pkt.append_item(enodeb_s1ap_ue_id);
	pkt.append_item(mme_s1ap_ue_id);

	g_sync.mlock(uectx_mux);
	pkt.append_item(ue_ctx[guti].s1_uteid_ul); //give your uplink id to target ran
	g_sync.munlock(uectx_mux);
	pkt.prepend_s1ap_hdr(7, pkt.len, pkt.s1ap_hdr.enodeb_s1ap_ue_id, pkt.s1ap_hdr.mme_s1ap_ue_id);
	SctpClient to_target_ran_client;
	to_target_ran_client.conn(t_ran_ip_addr, t_ran_port);
	to_target_ran_client.snd(pkt);
	cout<<"send to target ran done from amf"<<endl;

}
void Amf::handle_handover_completion(Packet pkt) {

	cout<<"handover completion \n";

	UdpClient upf_client;
	uint64_t guti;
	uint32_t s1_uteid_dl_ho; //ran 2 has sent its id, dl id for upf to send data
	uint32_t s1_uteid_ul;
	uint32_t s11_cteid_upf;
	bool res;

	upf_client.conn(g_amf_ip_addr, g_upf_s11_ip_addr, g_upf_s11_port);
	guti = get_guti(pkt);
	g_sync.mlock(uectx_mux);

	s11_cteid_upf = ue_ctx[guti].s11_cteid_upf;
	g_sync.munlock(uectx_mux);
	pkt.clear_pkt();
	pkt.append_item(1);//success marker

	pkt.prepend_gtp_hdr(2, 5, pkt.len, s11_cteid_upf);
	upf_client.snd(pkt);
	upf_client.rcv(pkt);

	//we will now return from here to source enb
	pkt.extract_gtp_hdr();
	pkt.extract_item(res);

	SctpClient to_source_ran_client;
	to_source_ran_client.conn(s_ran_ip_addr.c_str(), s_ran_port);

	if (res) {

		pkt.clear_pkt();
		pkt.append_item(res);
		//9 for msg for source ran with the indirect tunnel id.
		pkt.prepend_s1ap_hdr(9, pkt.len, pkt.s1ap_hdr.enodeb_s1ap_ue_id, pkt.s1ap_hdr.mme_s1ap_ue_id);
		to_source_ran_client.snd(pkt);

	}
	cout << "handle_handover_completion:" << " handover setup completed" << endl;
}
void Amf::teardown_indirect_tunnel(Packet pkt) {

	cout<<"tear down at amf \n";
	bool res;
	UdpClient upf_client;
	uint64_t guti;
	uint32_t s1_uteid_dl_ho; //ran 2 has sent its id, dl id for upf to send data
	uint32_t s1_uteid_ul;
	uint32_t s11_cteid_upf;

	uint32_t s1_uteid_ul_ho; 
	pkt.extract_item(s1_uteid_ul_ho);

	//tear down this teid from upf

	upf_client.conn(g_amf_ip_addr, g_upf_s11_ip_addr, g_upf_s11_port);
	guti = get_guti(pkt);

	g_sync.mlock(uectx_mux);
	s11_cteid_upf = ue_ctx[guti].s11_cteid_upf;
	g_sync.munlock(uectx_mux);

	pkt.clear_pkt();
	pkt.append_item(s1_uteid_ul_ho);

	pkt.prepend_gtp_hdr(2, 6, pkt.len, s11_cteid_upf);

	upf_client.snd(pkt);
	upf_client.rcv(pkt);
	pkt.extract_gtp_hdr();
	pkt.extract_item(res);

	if(res)
		cout << "tear down complted:" << " " << endl;
}

void Amf::handle_detach(int conn_fd, Packet pkt, int worker_id) {

	uint64_t guti;
	uint64_t k_nas_enc;
	uint64_t k_nas_int;
	uint64_t ksi_asme;
	uint64_t detach_type;
	uint64_t tai;
	uint32_t s11_cteid_upf;
	uint8_t eps_bearer_id;
	bool res;

	guti = get_guti(pkt);
	if (guti == 0) {
		TRACE(cout << "amf_handledetach:" << " zero guti " << pkt.s1ap_hdr.mme_s1ap_ue_id << " " << pkt.len << ": " << guti << endl;)
		g_utils.handle_type1_error(-1, "Zero guti: amf_handledetach");
	}
	//
	// g_sync.mlock(uectx_mux);
	// k_nas_enc = ue_ctx[guti].k_nas_enc;
	// k_nas_int = ue_ctx[guti].k_nas_int;
	// eps_bearer_id = ue_ctx[guti].eps_bearer_id;
	// tai = ue_ctx[guti].tai;
	// s11_cteid_upf = ue_ctx[guti].s11_cteid_upf;
	// g_sync.munlock(uectx_mux);
	//

	Json::Value requestPkt, jsonRes;
	requestPkt["guti"] = to_string(guti);

	bool parsingSuccessful = send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/16",
		requestPkt, jsonRes
	);

	if(!parsingSuccessful) {
		// TODO: handle error
		cout << "amf_handledetach: Response received from UDM parsing failed" << endl;
	}

	k_nas_enc = jsonRes["k_nas_enc"].asUInt64();
	k_nas_int = jsonRes["k_nas_int"].asUInt64();
	eps_bearer_id = jsonRes["eps_bearer_id"].asUInt();
	tai = jsonRes["tai"].asUInt64();
	s11_cteid_upf = jsonRes["s11_cteid_upf"].asUInt();

	TRACE(cout << "amf_handledetach:" << " detach req received: " << pkt.len << ": " << guti << endl;)

	if (HMAC_ON) {
		res = g_integrity.hmac_check(pkt, k_nas_int);
		if (!res)
		{
			TRACE(cout << "amf_handledetach:"
					   << " hmac detach failure: " << guti << endl;)
			g_utils.handle_type1_error(-1, "hmac failure: amf_handledetach");
		}
	}
	if (ENC_ON) {
		g_crypt.dec(pkt, k_nas_enc);
	}
	pkt.extract_item(guti); /* It should be the same as that found in the first step */
	pkt.extract_item(ksi_asme);
	pkt.extract_item(detach_type);

	TRACE(cout << "amf_handledetach:" << " detach request sent to smf: " << guti << endl;)

	requestPkt.clear();
	jsonRes.clear();
	requestPkt["guti"] = to_string(guti);
	requestPkt["eps_bearer_id"] = to_string(eps_bearer_id);
	requestPkt["tai"] = to_string(tai);

	parsingSuccessful = send_and_receive(
		smf_amf_ip_addr, 
		SMF_AMF_PORT_START_RANGE + worker_id, 
		"/Nsmf_PDUSession/ReleaseSMContext",
		requestPkt, jsonRes
	);

	TRACE(cout << "amf_handledetach :: Received from SMF : " << jsonRes.asString() << endl;)
	TRACE(cout << "amf_handledetach:" << " detach response received from smf: " << guti << endl;)

	// TODO handle respone errors
	if(!parsingSuccessful) {
		TRACE(cout << "ERROR :: amf_handledetach:" << " handle detach failure: JSON parsing failed" << endl;)
		return;
	}

	res = false; 		// Default res failed
	if(jsonRes.isMember("res")) {
		res = jsonRes["res"].asBool();
	}

	if (!res) {
		TRACE(cout << "ERROR :: amf_handledetach:" << " detach failure: " << guti << endl;)
		return;
	}

	pkt.clear_pkt();
	pkt.append_item(res);

	if (ENC_ON) {
		g_crypt.enc(pkt, k_nas_enc);
	}
	if (HMAC_ON) {
		g_integrity.add_hmac(pkt, k_nas_int);
	}
	pkt.prepend_s1ap_hdr(5, pkt.len, pkt.s1ap_hdr.enodeb_s1ap_ue_id, pkt.s1ap_hdr.mme_s1ap_ue_id);
	server.snd(conn_fd, pkt);
	TRACE(cout << "amf_handledetach:" << " detach complete sent to ue: " << pkt.len << ": " << guti << endl;)

	rem_itfid(pkt.s1ap_hdr.mme_s1ap_ue_id);
	rem_uectx(guti);
	TRACE(cout << "amf_handledetach:" << " ue entry removed: " << guti << endl;)
	TRACE(cout << "amf_handledetach:" << " detach successful: " << guti << endl;)
}


void Amf::set_upf_info(uint64_t guti, int worker_id) {
	//
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].upf_smf_port = g_upf_smf_port;
	ue_ctx[guti].upf_smf_ip_addr = g_upf_smf_ip_addr;
	g_sync.munlock(uectx_mux);


	Json::Value requestPkt, jsonRes;
	requestPkt["guti"] = to_string(guti);
	requestPkt["g_upf_smf_port"] = to_string(g_upf_smf_port);
	requestPkt["g_upf_smf_ip_addr"] = g_upf_smf_ip_addr;

	send_and_receive(
		g_udm_ip_addr,
		UDM_PORT_START_RANGE + worker_id,
		"/Nudm_UECM/17",
		requestPkt, jsonRes
	);
}

uint64_t Amf::get_guti(Packet pkt) {
	uint64_t mme_s1ap_ue_id;
	uint64_t guti;

	guti = 0;
	mme_s1ap_ue_id = pkt.s1ap_hdr.mme_s1ap_ue_id;
	g_sync.mlock(s1amfid_mux);
	if (s1amf_id.find(mme_s1ap_ue_id) != s1amf_id.end()) {
		guti = s1amf_id[mme_s1ap_ue_id];
	}
	g_sync.munlock(s1amfid_mux);
	return guti;
}

void Amf::rem_itfid(uint32_t mme_s1ap_ue_id) {
	g_sync.mlock(s1amfid_mux);
	s1amf_id.erase(mme_s1ap_ue_id);
	g_sync.munlock(s1amfid_mux);
}

void Amf::rem_uectx(uint64_t guti) {
	g_sync.mlock(uectx_mux);
	ue_ctx.erase(guti);
	g_sync.munlock(uectx_mux);
}

Amf::~Amf() {

}

