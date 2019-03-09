#include "udm.h"
#include "discover.h"

using namespace std;
using namespace std::chrono;
using namespace ppconsul;
using ppconsul::Consul;

string g_udm_ip_addr = "";
int g_udm_port = G_UDM_PORT;

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

	Consul consul(CONSUL_ADDR);
	agent::Agent consulAgent(consul);

	serviceRegister("udm", consulAgent);
	g_udm_ip_addr = getMyIPAddress();
}

void Udm::clrstl() {
ue_ctx.clear();
}

void Udm::handle_mysql_conn() {
    g_sync.mlock(mysql_client_mux);
    mysql_client.conn();
    g_sync.munlock(mysql_client_mux);
}

void Udm::get_autn_info(int conn_fd, Packet &pkt) {
    uint64_t imsi;
    uint64_t key;
    uint64_t rand_num;
    pkt.extract_item(imsi);
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
    pkt.clear_pkt();
    pkt.append_item(key);
    pkt.append_item(rand_num);
    pkt.prepend_diameter_hdr(1,pkt.len);
    server.snd(conn_fd,pkt);
    TRACE(cout<<"udm_get_autn_info: "<<"response sent to ausf"<<endl;)
}

void Udm::set_loc_info(int conn_fd, Packet &pkt)
{
    uint64_t imsi;
    uint32_t mmei;

    pkt.extract_item(imsi);
    pkt.extract_item(mmei);

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
}

void Udm::update_info_amf_initial_attach(int connfd, Packet &pkt)
{
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

	pkt.extract_item(guti);
	pkt.extract_item(xres);
	pkt.extract_item(k_asme);
	pkt.extract_item(ksi_asme);
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].xres = xres;
	ue_ctx[guti].k_asme = k_asme;
	ue_ctx[guti].ksi_asme = ksi_asme;
	g_sync.munlock(uectx_mux);
	TRACE(cout<<"update amf_initial_attach UE CTX complete"<<endl;)
}

void Udm::update_info_amf_initial_attach_init(int connfd, Packet &pkt)
{
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
	pkt.extract_item(guti);
	pkt.extract_item(imsi);
	pkt.extract_item(enodeb_s1ap_ue_id);
	pkt.extract_item(mme_s1ap_ue_id);
	pkt.extract_item(tai);
	pkt.extract_item(nw_capability);

	// now will just update the ue_contxt for the guti appended in the packet.
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].imsi = imsi;
	ue_ctx[guti].enodeb_s1ap_ue_id = enodeb_s1ap_ue_id;
	ue_ctx[guti].mme_s1ap_ue_id = mme_s1ap_ue_id;
	ue_ctx[guti].tai = tai;
	ue_ctx[guti].nw_capability = nw_capability;
	g_sync.munlock(uectx_mux);

	TRACE(cout<<"Amf_initial_attach INIT ue contxt updated: "<<endl;)
}

void Udm::handle_autn_ue_ctx_request(int connfd, Packet &pkt)
{
	uint64_t guti;
	uint64_t xres;
	pkt.extract_item(guti);
	g_sync.mlock(uectx_mux);
	xres = ue_ctx[guti].xres;
	g_sync.munlock(uectx_mux);
	pkt.clear_pkt();
	pkt.append_item(xres);
	server.snd(connfd,pkt);
	TRACE(cout<<"sent the handle autn request to the amf"<<endl;)
}

void Udm::ue_ctx_request_security_mode_cmd(int connfd, Packet &pkt)
{
	uint64_t guti;
	uint64_t ksi_asme;
	uint16_t nw_capability;
	uint64_t nas_enc_algo;
	uint64_t nas_int_algo;
	uint64_t k_nas_enc;
	uint64_t k_nas_int;

	pkt.extract_item(guti);
	pkt.clear_pkt();
	g_sync.mlock(uectx_mux);
	pkt.append_item(ue_ctx[guti].ksi_asme);
	pkt.append_item(ue_ctx[guti].nw_capability);
	pkt.append_item(ue_ctx[guti].nas_enc_algo);
	pkt.append_item(ue_ctx[guti].nas_int_algo);
	pkt.append_item(ue_ctx[guti].k_nas_enc);
	pkt.append_item(ue_ctx[guti].k_nas_int);
	g_sync.munlock(uectx_mux);
	server.snd(connfd,pkt);
	TRACE(cout<<"UE CTX information sent to the amf"<<endl;)

}

void Udm::ue_ctx_request_set_crypt_context(int connfd, Packet &pkt)
{

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
	uint64_t num_autn_vectors;
	uint64_t guti;
	uint64_t nas_enc_algo;
	uint64_t nas_int_algo;
	uint64_t k_nas_enc;
	uint64_t k_nas_int;
	uint64_t count;
	uint64_t bearer;
	uint64_t dir;

	pkt.extract_item(guti);
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].nas_enc_algo = 1;
	ue_ctx[guti].k_nas_enc = ue_ctx[guti].k_asme + ue_ctx[guti].nas_enc_algo + ue_ctx[guti].count + ue_ctx[guti].bearer + ue_ctx[guti].dir;
	g_sync.munlock(uectx_mux);
	TRACE(cout<<"ue ctx set crypt updation done"<<endl;)

}

void Udm::ue_ctx_update_set_integrity_context(int connfd, Packet &pkt)
{
	uint64_t guti;
	pkt.extract_item(guti);
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].nas_int_algo = 1;
	ue_ctx[guti].k_nas_int = ue_ctx[guti].k_asme + ue_ctx[guti].nas_int_algo + ue_ctx[guti].count + ue_ctx[guti].bearer + ue_ctx[guti].dir;
	g_sync.munlock(uectx_mux);
	TRACE(cout<<"ue ctx updated for set integrity context"<<endl;)
}

void Udm::ue_ctx_request_handle_security_mode_complete(int connfd, Packet &pkt)
{
	uint64_t guti;
	uint64_t k_nas_enc;
	uint64_t k_nas_int;
	pkt.extract_item(guti);
	g_sync.mlock(uectx_mux);
	k_nas_enc = ue_ctx[guti].k_nas_enc;
	k_nas_int = ue_ctx[guti].k_nas_int;
	g_sync.munlock(uectx_mux);
	pkt.clear_pkt();
	pkt.append_item(k_nas_enc);
	pkt.append_item(k_nas_int);
	server.snd(connfd, pkt);
	TRACE(cout<<"Packet sent to amf for handle_security_complete"<<endl;)
}

void Udm::ue_ctx_request_handle_location_update(int connfd, Packet &pkt)
{
	uint64_t imsi;
	uint64_t guti;
	pkt.extract_item(guti);
	g_sync.mlock(uectx_mux);
	imsi = ue_ctx[guti].imsi;
	g_sync.munlock(uectx_mux);
	pkt.clear_pkt();
	pkt.append_item(imsi);
	server.snd(connfd,pkt);
}

void Udm::ue_ctx_request_handle_create_session(int connfd, Packet &pkt)
{
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

	pkt.extract_item(guti);
	pkt.extract_item(s11_cteid_amf);
	pkt.extract_item(eps_bearer_id);
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].s11_cteid_amf = s11_cteid_amf;
	ue_ctx[guti].eps_bearer_id = eps_bearer_id;
	pkt.clear_pkt();
	pkt.append_item(ue_ctx[guti].s11_cteid_amf);
	pkt.append_item(ue_ctx[guti].imsi);
	pkt.append_item(ue_ctx[guti].eps_bearer_id);
	pkt.append_item(ue_ctx[guti].upf_smf_ip_addr);
	pkt.append_item(ue_ctx[guti].upf_smf_port);
	pkt.append_item(ue_ctx[guti].apn_in_use);
	pkt.append_item(ue_ctx[guti].tai);
	g_sync.munlock(uectx_mux);
	server.snd(connfd,pkt);
	TRACE(cout<<"UE CTX for handle create session sent"<<endl;)
}

void Udm::ue_ctx_update_handle_craete_session(int connfd, Packet &pkt)
{
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

	pkt.extract_item(guti);
	g_sync.mlock(uectx_mux);
	pkt.extract_item(ue_ctx[guti].ip_addr);
	pkt.extract_item(ue_ctx[guti].s11_cteid_upf);
	pkt.extract_item(ue_ctx[guti].s1_uteid_ul);
	ue_ctx[guti].tai_list.clear();
	ue_ctx[guti].tai_list.push_back(ue_ctx[guti].tai);
	pkt.extract_item(ue_ctx[guti].tau_timer);
	ue_ctx[guti].e_rab_id = ue_ctx[guti].eps_bearer_id;
	ue_ctx[guti].k_enodeb = ue_ctx[guti].k_asme;
	g_sync.munlock(uectx_mux);
	tai_list_size = 1;
	pkt.clear_pkt();
	pkt.append_item(ue_ctx[guti].e_rab_id);
	pkt.append_item(ue_ctx[guti].k_enodeb);
	pkt.append_item(ue_ctx[guti].nw_capability);
	pkt.append_item(tai_list_size);
	pkt.append_item(ue_ctx[guti].tai_list);
	pkt.extract_item(ue_ctx[guti].tau_timer);
	pkt.extract_item(ue_ctx[guti].k_nas_enc);
	pkt.extract_item(ue_ctx[guti].k_nas_int);
	server.snd(connfd,pkt);

	TRACE(cout<<"UE CTX update handle create session has sent to AMF"<<endl;)
}

void Udm::ue_ctx_request_handle_attach_complete(int connfd, Packet &pkt)
{
	uint64_t guti;
	uint64_t k_nas_enc;
	uint64_t k_nas_int;
	uint32_t s1_uteid_dl;
	uint8_t eps_bearer_id;
	pkt.extract_item(guti);
	pkt.clear_pkt();
	g_sync.mlock(uectx_mux);
	pkt.append_item(ue_ctx[guti].k_nas_enc);
	pkt.append_item(ue_ctx[guti].k_nas_int);
	g_sync.munlock(uectx_mux);
	server.snd(connfd,pkt);
	TRACE(cout<<"UE CTX for handle attach complete send to amf"<<endl;)
}

void Udm::ue_ctx_update_handle_attach_complete(int connfd, Packet &pkt)
{
	uint64_t guti;
	uint64_t k_nas_enc;
	uint64_t k_nas_int;
	uint32_t s1_uteid_dl;
	uint8_t eps_bearer_id;
	pkt.extract_item(guti);
	pkt.extract_item(s1_uteid_dl);
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].s1_uteid_dl = s1_uteid_dl;
	ue_ctx[guti].emm_state = 1;
	g_sync.munlock(uectx_mux);
	TRACE(cout<<"UE CTX UPADTE from amf handle attach complete"<<endl;)
}

void Udm::ue_ctx_request_handle_modify_bearer(int connfd, Packet &pkt)
{
	uint64_t guti;
	pkt.extract_item(guti);
	pkt.clear_pkt();
	g_sync.mlock(uectx_mux);
	ue_ctx[guti].ecm_state = 1;
	pkt.append_item(ue_ctx[guti].eps_bearer_id);
	pkt.append_item(ue_ctx[guti].s1_uteid_dl);
	pkt.append_item(ue_ctx[guti].s11_cteid_upf);
	g_sync.munlock(uectx_mux);
	server.snd(connfd, pkt);
	TRACE(cout<<"UE CTX Request from handle modify bearer complete"<<endl;)
}

void Udm::ue_ctx_request_handle_detach(int connfd, Packet &pkt)
{
	uint64_t guti;
	pkt.extract_item(guti);
	pkt.clear_pkt();
	g_sync.mlock(uectx_mux);
	pkt.append_item(ue_ctx[guti].k_nas_enc);
	pkt.append_item(ue_ctx[guti].k_nas_int);
	pkt.append_item(ue_ctx[guti].eps_bearer_id);
	pkt.append_item(ue_ctx[guti].tai);
	pkt.append_item(ue_ctx[guti].s11_cteid_upf);
	g_sync.munlock(uectx_mux);
	server.snd(connfd,pkt);
	TRACE(cout<<"UE CTX Request from amf handle detach"<<endl;)
}

void Udm::ue_ctx_update_set_upf_info(int connfd, Packet &pkt)
{
	uint64_t guti;
	pkt.extract_item(guti);
	g_sync.mlock(uectx_mux);
	pkt.extract_item(ue_ctx[guti].upf_smf_port);
	pkt.extract_item(ue_ctx[guti].upf_smf_ip_addr);
	g_sync.munlock(uectx_mux);
}

void Udm::ue_ctx_request_smf_handle_create_session(int connfd, Packet &pkt)
{
	uint64_t guti;
	pkt.extract_item(guti);
	g_sync.mlock(uectx_mux);
	pkt.extract_item(ue_ctx[guti].s11_cteid_mme);
	pkt.extract_item(ue_ctx[guti].eps_bearer_id);
	pkt.extract_item(ue_ctx[guti].imsi);
	pkt.extract_item(ue_ctx[guti].apn_in_use);
	pkt.extract_item(ue_ctx[guti].tai);

	pkt.clear_pkt();
	pkt.append_item(ue_ctx[guti].s11_cteid_mme);
	pkt.append_item(ue_ctx[guti].eps_bearer_id);
	pkt.append_item(ue_ctx[guti].imsi);
	pkt.append_item(ue_ctx[guti].apn_in_use);
	pkt.append_item(ue_ctx[guti].tai);
	g_sync.munlock(uectx_mux);
	server.snd(connfd, pkt);
	TRACE(cout<<"Response sent to request from smf handle create session"<<endl;)
}

void Udm::ue_ctx_update_smf_handle_create_session(int connfd, Packet &pkt)
{
	uint64_t guti;
	pkt.extract_item(guti);
	g_sync.mlock(uectx_mux);
	pkt.extract_item(ue_ctx[guti].ip_addr);
	pkt.extract_item(ue_ctx[guti].s11_cteid_sgw);
	pkt.extract_item(ue_ctx[guti].s1_uteid_ul);
	ue_ctx[guti].tai_list.clear();
	ue_ctx[guti].tai_list.push_back(ue_ctx[guti].tai);
	pkt.extract_item(ue_ctx[guti].tau_timer);
	ue_ctx[guti].e_rab_id = ue_ctx[guti].eps_bearer_id;

	pkt.clear_pkt();
	pkt.append_item(ue_ctx[guti].e_rab_id);
	pkt.append_item(ue_ctx[guti].k_enodeb);
	pkt.append_item(ue_ctx[guti].tai_list);
	pkt.append_item(ue_ctx[guti].tau_timer);
	g_sync.munlock(uectx_mux);
	server.snd(connfd, pkt);
	TRACE(cout<<"Response sent to update smf handle create session"<<endl;)
}

void Udm::ue_ctx_request_smf_handle_modify_bearer(int connfd, Packet &pkt)
{
	uint64_t guti;
	pkt.extract_item(guti);
	pkt.clear_pkt();
	pkt.append_item(ue_ctx[guti].eps_bearer_id);
	pkt.append_item(ue_ctx[guti].s1_uteid_dl);
	pkt.append_item(ue_ctx[guti].s11_cteid_sgw);
	server.snd(connfd, pkt);
	TRACE(cout<<"Response sent to request smf handle modify bearer"<<endl;)
}

void Udm::ue_ctx_request_smf_handle_detach(int connfd, Packet &pkt)
{
	uint64_t guti;
	pkt.extract_item(guti);
	pkt.clear_pkt();
	pkt.append_item(ue_ctx[guti].s11_cteid_sgw);
	server.snd(connfd, pkt);
	TRACE(cout<<"Response sent to request from handle detach"<<endl;)
}



Udm::~Udm(){
}