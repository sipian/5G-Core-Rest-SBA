#include "udm.h"
#include "discover.h"
#include "rest_utils.h"
using namespace std;
using namespace std::chrono;

// string g_udm_ip_addr = "172.26.0.3";
string g_udm_ip_addr = resolve_host("udm");
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


Udm::Udm(){
	clrstl();
    g_sync.mux_init(mysql_client_mux);
}

void Udm::clrstl() {
ue_ctx.clear();
}

void Udm::handle_mysql_conn() {
    g_sync.mlock(mysql_client_mux);
    mysql_client.conn();
    g_sync.munlock(mysql_client_mux);
}

std::string Udm::get_autn_info(Json::Value &jsonPkt) {
    uint64_t imsi;
    uint64_t key;
    uint64_t rand_num;
	imsi = jsonPkt["imsi"].asUInt64();
	MYSQL_RES *query_res;
	MYSQL_ROW query_res_row;
	int i;
	int num_fields;
	string query;

	TRACE(cout<<"Inside UDM will display the time for query execution"<<endl;)
	// auto start = high_resolution_clock::now();

	query_res = NULL;
	query = "select key_id, rand_num from autn_info where imsi = " + to_string(imsi);
	TRACE(cout << "ausf_getautninfo:" << query << endl;)
	g_sync.mlock(mysql_client_mux);
	mysql_client.handle_query(query, &query_res);
	g_sync.munlock(mysql_client_mux);
	num_fields = mysql_num_fields(query_res);
	TRACE(cout << "ausf_getautninfo:" << " fetched" << endl;)
	query_res_row = mysql_fetch_row(query_res);
	if (query_res_row == 0) {
		g_utils.handle_type1_error(-1, "mysql_fetch_row error: ausf_getautninfo");
	}
	for (i = 0; i < num_fields; i++) {
		string query_res_field;

		query_res_field = query_res_row[i];
		if (i == 0) {
			key = stoull(query_res_field);
		}
		else {
			rand_num = stoull(query_res_field);
		}
	}
	mysql_free_result(query_res);
	// auto stop = high_resolution_clock::now();
	// auto duration = duration_cast<microseconds>(stop-start);
	// TRACE(cout<<"Get autn info Time taken is: "<<duration.count()<<endl;)

    // TRACE(cout<<"udm_get_autn_info: "<<"response sent to ausf"<<endl;)
	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;
	resJsonPkt["key"] = touint64(key);
	resJsonPkt["rand_num"] = touint64(rand_num);
	return fastWriter.write(resJsonPkt);
}

std::string Udm::set_loc_info(Json::Value &jsonPkt) {
    uint64_t imsi = jsonPkt["imsi"].asUInt64();
    uint32_t mmei = jsonPkt["mmei"].asUInt();

    MYSQL_RES *query_res;
	string query;
	auto start = high_resolution_clock::now();
	query_res = NULL;
	query = "delete from loc_info where imsi = " + to_string(imsi);
	TRACE(cout << "ausf_setlocinfo:" << " " << query << endl;)
	g_sync.mlock(mysql_client_mux);
	mysql_client.handle_query(query, &query_res);
	g_sync.munlock(mysql_client_mux);
	query = "insert into loc_info values(" + to_string(imsi) + ", " + to_string(mmei) + ")";
	TRACE(cout << "ausf_setlocinfo:" << " " << query << endl;)
	g_sync.mlock(mysql_client_mux);
	mysql_client.handle_query(query, &query_res);
	g_sync.munlock(mysql_client_mux);
	mysql_free_result(query_res);
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop-start);
	cout<<"set loc info time taken is"<<duration.count()<<endl;
	return "{}";
}

std::string Udm::update_info_amf_initial_attach(Json::Value &jsonPkt) {
	uint64_t ksi_asme = jsonPkt["ksi_asme"].asUInt64();
	uint64_t xres = jsonPkt["xres"].asUInt64();
	uint64_t k_asme = jsonPkt["k_asme"].asUInt64();
	uint64_t guti = jsonPkt["guti"].asUInt64();

	g_sync.mlock(uectx_mux);
	ue_ctx[guti].xres = xres;
	ue_ctx[guti].k_asme = k_asme;
	ue_ctx[guti].ksi_asme = ksi_asme;
	g_sync.munlock(uectx_mux);
	TRACE(cout<<"update amf_initial_attach UE CTX complete"<<endl;)
	return "{}";
}

std::string Udm::update_info_amf_initial_attach_init(Json::Value &jsonPkt) {
	uint64_t imsi = jsonPkt["imsi"].asUInt64();
	uint64_t tai = jsonPkt["tai"].asUInt64();
	uint16_t nw_capability = jsonPkt["nw_capability"].asUInt();
	uint32_t enodeb_s1ap_ue_id = jsonPkt["enodeb_s1ap_ue_id"].asUInt();
	uint32_t mme_s1ap_ue_id = jsonPkt["mme_s1ap_ue_id"].asUInt();
	uint64_t guti = jsonPkt["guti"].asUInt64();

	// now will just update the ue_contxt for the guti appended in the packet.
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].imsi = imsi;
	ue_ctx[guti].enodeb_s1ap_ue_id = enodeb_s1ap_ue_id;
	ue_ctx[guti].mme_s1ap_ue_id = mme_s1ap_ue_id;
	ue_ctx[guti].tai = tai;
	ue_ctx[guti].nw_capability = nw_capability;
	g_sync.munlock(uectx_mux);

	TRACE(cout<<"Amf_initial_attach INIT ue contxt updated: "<<endl;)
	return "{}";
}

std::string Udm::handle_autn_ue_ctx_request(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();
	uint64_t xres;
	g_sync.mlock(uectx_mux);
	xres = ue_ctx[guti].xres;
	g_sync.munlock(uectx_mux);

	// TRACE(cout<<"sent the handle autn request to the amf"<<endl;)
	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;
	resJsonPkt["xres"] = touint64(xres);
	return fastWriter.write(resJsonPkt);
}

std::string Udm::ue_ctx_request_security_mode_cmd(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;
	g_sync.mlock(uectx_mux);

	resJsonPkt["ksi_asme"] = touint64(ue_ctx[guti].ksi_asme);
	resJsonPkt["nw_capability"] = touint(ue_ctx[guti].nw_capability);
	resJsonPkt["nas_enc_algo"] = touint64(ue_ctx[guti].nas_enc_algo);
	resJsonPkt["nas_int_algo"] = touint64(ue_ctx[guti].nas_int_algo);
	resJsonPkt["k_nas_enc"] = touint64(ue_ctx[guti].k_nas_enc);
	resJsonPkt["k_nas_int"] = touint64(ue_ctx[guti].k_nas_int);

	g_sync.munlock(uectx_mux);

	// TRACE(cout<<"UE CTX information sent to the amf"<<endl;)
	return fastWriter.write(resJsonPkt);
}

std::string Udm::ue_ctx_request_set_crypt_context(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	g_sync.mlock(uectx_mux);
	ue_ctx[guti].nas_enc_algo = 1;
	ue_ctx[guti].k_nas_enc = ue_ctx[guti].k_asme + ue_ctx[guti].nas_enc_algo + ue_ctx[guti].count + ue_ctx[guti].bearer + ue_ctx[guti].dir;
	g_sync.munlock(uectx_mux);
	
	TRACE(cout<<"ue ctx set crypt updation done"<<endl;)
	return "{}";
}

std::string Udm::ue_ctx_update_set_integrity_context(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	g_sync.mlock(uectx_mux);
	ue_ctx[guti].nas_int_algo = 1;
	ue_ctx[guti].k_nas_int = ue_ctx[guti].k_asme + ue_ctx[guti].nas_int_algo + ue_ctx[guti].count + ue_ctx[guti].bearer + ue_ctx[guti].dir;
	g_sync.munlock(uectx_mux);

	TRACE(cout<<"ue ctx updated for set integrity context"<<endl;)
	return "{}";
}

std::string Udm::ue_ctx_request_handle_security_mode_complete(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	uint64_t k_nas_enc;
	uint64_t k_nas_int;

	g_sync.mlock(uectx_mux);
	k_nas_enc = ue_ctx[guti].k_nas_enc;
	k_nas_int = ue_ctx[guti].k_nas_int;
	g_sync.munlock(uectx_mux);
	
	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;
	resJsonPkt["k_nas_enc"] = touint64(k_nas_enc);
	resJsonPkt["k_nas_int"] = touint64(k_nas_int);
	return fastWriter.write(resJsonPkt);
	// TRACE(cout<<"Packet sent to amf for handle_security_complete"<<endl;)
}

std::string Udm::ue_ctx_request_handle_location_update(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	uint64_t imsi;

	g_sync.mlock(uectx_mux);
	imsi = ue_ctx[guti].imsi;
	g_sync.munlock(uectx_mux);

	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;
	resJsonPkt["imsi"] = touint64(imsi);
	return fastWriter.write(resJsonPkt);
}

std::string Udm::ue_ctx_request_handle_create_session(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();
	uint32_t s11_cteid_amf = jsonPkt["s11_cteid_amf"].asUInt();
	uint8_t eps_bearer_id = jsonPkt["eps_bearer_id"].asUInt();

	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].s11_cteid_amf = s11_cteid_amf;
	ue_ctx[guti].eps_bearer_id = eps_bearer_id;

	resJsonPkt["s11_cteid_amf"] = touint(ue_ctx[guti].s11_cteid_amf);
	resJsonPkt["imsi"] = touint64(ue_ctx[guti].imsi);
	resJsonPkt["eps_bearer_id"] = touint(ue_ctx[guti].eps_bearer_id);
	resJsonPkt["upf_smf_ip_addr"] = ue_ctx[guti].upf_smf_ip_addr;
	resJsonPkt["upf_smf_port"] = toint(ue_ctx[guti].upf_smf_port);
	resJsonPkt["apn_in_use"] = touint64(ue_ctx[guti].apn_in_use);
	resJsonPkt["tai"] = touint64(ue_ctx[guti].tai);

	g_sync.munlock(uectx_mux);
	
	return fastWriter.write(resJsonPkt);
	// TRACE(cout<<"UE CTX for handle create session sent"<<endl;)
}

std::string Udm::ue_ctx_update_handle_craete_session(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();
	
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].ip_addr = jsonPkt["ue_ip_addr"].asString();
	ue_ctx[guti].s11_cteid_upf = jsonPkt["s11_cteid_upf"].asUInt();
	ue_ctx[guti].s1_uteid_ul = jsonPkt["s1_uteid_ul"].asUInt();

	ue_ctx[guti].tai_list.clear();
	ue_ctx[guti].tai_list.push_back(ue_ctx[guti].tai);

	ue_ctx[guti].tau_timer = jsonPkt["tau_timer"].asUInt64();
	ue_ctx[guti].e_rab_id = ue_ctx[guti].eps_bearer_id;
	ue_ctx[guti].k_enodeb = ue_ctx[guti].k_asme;
	g_sync.munlock(uectx_mux);
	
	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;

	resJsonPkt["e_rab_id"] = touint(ue_ctx[guti].e_rab_id);
	resJsonPkt["k_enodeb"] = touint64(ue_ctx[guti].k_enodeb);
	resJsonPkt["nw_capability"] = touint(ue_ctx[guti].nw_capability);

	resJsonPkt["tai_list"] = Json::arrayValue;
	resJsonPkt["tai_list"].append(touint64(ue_ctx[guti].tai_list[0]));
	
	resJsonPkt["tau_timer"] = touint64(ue_ctx[guti].tau_timer);
	resJsonPkt["k_nas_enc"] = touint64(ue_ctx[guti].k_nas_enc);
	resJsonPkt["k_nas_int"] = touint64(ue_ctx[guti].k_nas_int);

	return fastWriter.write(resJsonPkt);
	// TRACE(cout<<"UE CTX update handle create session has sent to AMF"<<endl;)
}

std::string Udm::ue_ctx_request_handle_attach_complete(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	uint64_t k_nas_enc;
	uint64_t k_nas_int;
	uint32_t s1_uteid_dl;
	uint8_t eps_bearer_id;

	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;

	g_sync.mlock(uectx_mux);

	resJsonPkt["k_nas_enc"] = touint64(ue_ctx[guti].k_nas_enc);
	resJsonPkt["k_nas_int"] = touint64(ue_ctx[guti].k_nas_int);

	g_sync.munlock(uectx_mux);

	return fastWriter.write(resJsonPkt);
	// TRACE(cout<<"UE CTX for handle attach complete send to amf"<<endl;)
}

std::string Udm::ue_ctx_update_handle_attach_complete(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();
	uint32_t s1_uteid_dl = jsonPkt["s1_uteid_dl"].asUInt();

	g_sync.mlock(uectx_mux);
	ue_ctx[guti].s1_uteid_dl = s1_uteid_dl;
	ue_ctx[guti].emm_state = 1;
	g_sync.munlock(uectx_mux);
	TRACE(cout<<"UE CTX UPADTE from amf handle attach complete"<<endl;)
	return "{}";
}

std::string Udm::ue_ctx_request_handle_modify_bearer(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;

	g_sync.mlock(uectx_mux);

	ue_ctx[guti].ecm_state = 1;
	resJsonPkt["eps_bearer_id"] = touint(ue_ctx[guti].eps_bearer_id);
	resJsonPkt["s1_uteid_dl"] = touint(ue_ctx[guti].s1_uteid_dl);
	resJsonPkt["s11_cteid_upf"] = touint(ue_ctx[guti].s11_cteid_upf);

	g_sync.munlock(uectx_mux);

	return fastWriter.write(resJsonPkt);
	// TRACE(cout<<"UE CTX Request from handle modify bearer complete"<<endl;)
}

std::string Udm::ue_ctx_request_handle_detach(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;

	g_sync.mlock(uectx_mux);

	resJsonPkt["k_nas_enc"] = touint64(ue_ctx[guti].k_nas_enc);
	resJsonPkt["k_nas_int"] = touint64(ue_ctx[guti].k_nas_int);
	resJsonPkt["eps_bearer_id"] = touint(ue_ctx[guti].eps_bearer_id);
	resJsonPkt["tai"] = touint64(ue_ctx[guti].tai);
	resJsonPkt["s11_cteid_upf"] = touint(ue_ctx[guti].s11_cteid_upf);
	
	g_sync.munlock(uectx_mux);

	return fastWriter.write(resJsonPkt);
	// TRACE(cout<<"UE CTX Request from amf handle detach"<<endl;)
}

std::string Udm::ue_ctx_update_set_upf_info(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	g_sync.mlock(uectx_mux);
	ue_ctx[guti].upf_smf_port = jsonPkt["upf_smf_port"].asInt();
	ue_ctx[guti].upf_smf_ip_addr = jsonPkt["upf_smf_ip_addr"].asString();
	g_sync.munlock(uectx_mux);
	return "{}";
}

std::string Udm::ue_ctx_request_smf_handle_create_session(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;

	g_sync.mlock(uectx_mux);

	ue_ctx[guti].s11_cteid_mme = jsonPkt["s11_cteid_mme"].asUInt();
	ue_ctx[guti].eps_bearer_id = jsonPkt["eps_bearer_id"].asUInt();
	ue_ctx[guti].imsi = jsonPkt["imsi"].asUInt64();
	ue_ctx[guti].apn_in_use = jsonPkt["apn_in_use"].asUInt64();
	ue_ctx[guti].tai = jsonPkt["tai"].asUInt64();

	resJsonPkt["s11_cteid_mme"] = touint(ue_ctx[guti].s11_cteid_mme);
	resJsonPkt["eps_bearer_id"] = touint(ue_ctx[guti].eps_bearer_id);
	resJsonPkt["imsi"] = touint64(ue_ctx[guti].imsi);
	resJsonPkt["apn_in_use"] = touint64(ue_ctx[guti].apn_in_use);
	resJsonPkt["tai"] = touint64(ue_ctx[guti].tai);

	g_sync.munlock(uectx_mux);
	
	return fastWriter.write(resJsonPkt);
	// TRACE(cout<<"Response sent to request from smf handle create session"<<endl;)
}

std::string Udm::ue_ctx_update_smf_handle_create_session(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;

	g_sync.mlock(uectx_mux);

	ue_ctx[guti].ip_addr = jsonPkt["ue_ip_addr"].asString();
	ue_ctx[guti].s11_cteid_sgw = jsonPkt["s11_cteid_sgw"].asUInt();
	ue_ctx[guti].s1_uteid_ul = jsonPkt["s1_uteid_ul"].asUInt();

	ue_ctx[guti].tai_list.clear();
	ue_ctx[guti].tai_list.push_back(ue_ctx[guti].tai);
	ue_ctx[guti].tau_timer = jsonPkt["tau_timer"].asUInt64();
	ue_ctx[guti].e_rab_id = ue_ctx[guti].eps_bearer_id;

	resJsonPkt["e_rab_id"] = touint(ue_ctx[guti].e_rab_id);
	resJsonPkt["k_enodeb"] = touint64(ue_ctx[guti].k_enodeb);

	resJsonPkt["tai_list"] = Json::arrayValue;
	for(auto val: ue_ctx[guti].tai_list) {
		resJsonPkt["tai_list"].append(touint64(val));
	}

	resJsonPkt["tau_timer"] = touint64(ue_ctx[guti].tau_timer);

	g_sync.munlock(uectx_mux);

	return fastWriter.write(resJsonPkt);
	// TRACE(cout<<"Response sent to update smf handle create session"<<endl;)
}

std::string Udm::ue_ctx_request_smf_handle_modify_bearer(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;

	resJsonPkt["eps_bearer_id"] = touint(ue_ctx[guti].eps_bearer_id);
	resJsonPkt["s1_uteid_dl"] = touint(ue_ctx[guti].s1_uteid_dl);
	resJsonPkt["s11_cteid_sgw"] = touint(ue_ctx[guti].s11_cteid_sgw);

	return fastWriter.write(resJsonPkt);
	// TRACE(cout<<"Response sent to request smf handle modify bearer"<<endl;)
}

std::string Udm::ue_ctx_request_smf_handle_detach(Json::Value &jsonPkt) {
	uint64_t guti = jsonPkt["guti"].asUInt64();

	Json::Value resJsonPkt;
	Json::FastWriter fastWriter;

	resJsonPkt["s11_cteid_sgw"] = touint(ue_ctx[guti].s11_cteid_sgw);

	return fastWriter.write(resJsonPkt);
	// TRACE(cout<<"Response sent to request from handle detach"<<endl;)
}



Udm::~Udm(){
}