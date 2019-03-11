#ifndef UDM_H
#define UDM_H

#include "diameter.h"
#include "gtp.h"
#include "mysql.h"
#include "network.h"
#include "packet.h"
#include "ports.h"
#include "ppconsul/agent.h"
#include "s1ap.h"
#include "sctp_server.h"
#include "sctp_client.h"
#include "sync.h"
#include "utils.h"
#include <string>
#include <jsoncpp/json/json.h>

extern string g_udm_ip_addr;
extern int g_udm_port;

class UeContext {
public:
	/* EMM state 
	 * 0 - Deregistered
	 * 1 - Registered */
	int emm_state; /* EPS Mobililty Management state */

	/* ECM state 
	 * 0 - Disconnected
	 * 1 - Connected 
	 * 2 - Idle */	 
	int ecm_state; /* EPS Connection Management state */

	/* UE id */
	uint64_t imsi; /* International Mobile Subscriber Identity */
	string ip_addr;
	uint32_t enodeb_s1ap_ue_id; /* eNodeB S1AP UE ID */
	uint32_t mme_s1ap_ue_id; /* AMF S1AP UE ID */

	/* UE location info */
	uint64_t tai; /* Tracking Area Identifier */
	vector<uint64_t> tai_list; /* Tracking Area Identifier list */
	uint64_t tau_timer; /* Tracking area update timer */

	/* UE security context */
	uint64_t ksi_asme; /* Key Selection Identifier for Access Security Management Entity */	
	uint64_t k_asme; /* Key for Access Security Management Entity */	
	uint64_t k_enodeb; /* Key for Access Stratum */	
	uint64_t k_nas_enc; /* Key for NAS Encryption / Decryption */
	uint64_t k_nas_int; /* Key for NAS Integrity check */
	uint64_t nas_enc_algo; /* Idenitifier of NAS Encryption / Decryption */
	uint64_t nas_int_algo; /* Idenitifier of NAS Integrity check */
	uint64_t count;
	uint64_t bearer;
	uint64_t dir;

	/* EPS info, EPS bearer info */
	uint64_t default_apn; /* Default Access Point Name */
	uint64_t apn_in_use; /* Access Point Name in Use */
	uint8_t eps_bearer_id; /* Evolved Packet System Bearer ID */
	uint8_t e_rab_id; /* Evolved Radio Access Bearer ID */	
	uint32_t s1_uteid_ul; /* S1 Userplane Tunnel Endpoint Identifier - Uplink */
	uint32_t s1_uteid_dl; /* S1 Userplane Tunnel Endpoint Identifier - Downlink */
	uint32_t s5_uteid_ul; /* S5 Userplane Tunnel Endpoint Identifier - Uplink */
	uint32_t s5_uteid_dl; /* S5 Userplane Tunnel Endpoint Identifier - Downlink */

	/* Authentication info */ 
	uint64_t xres;

	/* UE Operator network info */
	uint16_t nw_type;
	uint16_t nw_capability;

	/* PGW info */
	string upf_smf_ip_addr;
	int upf_smf_port;

	/* Control plane info */
	uint32_t s11_cteid_amf; /* S11 Controlplane Tunnel Endpoint Identifier - amf */
	uint32_t s11_cteid_upf; /* S11 Controlplane Tunnel Endpoint Identifier - UPF*/
	uint32_t s11_cteid_mme;
	uint32_t s11_cteid_sgw;

	UeContext();
	void init(uint64_t, uint32_t, uint32_t, uint64_t, uint16_t);
	~UeContext();
};


class Udm {
    private:
        pthread_mutex_t mysql_client_mux;
        unordered_map<uint64_t, UeContext> ue_ctx;   /* UE context table: guti -> UeContext */
        /* Lock parameters */
    	pthread_mutex_t uectx_mux; /* Handles ue_ctx */
        void clrstl();

    public:
        SctpServer server;
        MySql mysql_client;

        Udm();
        void handle_mysql_conn();
        std::string get_autn_info(Json::Value&);
        std::string set_loc_info(Json::Value&);
        std::string update_info_amf_initial_attach(Json::Value&);
        std::string update_info_amf_initial_attach_init(Json::Value&);
        std::string handle_autn_ue_ctx_request(Json::Value&);
        std::string ue_ctx_request_security_mode_cmd(Json::Value&);
        std::string ue_ctx_request_set_crypt_context(Json::Value&);
        std::string ue_ctx_update_set_integrity_context(Json::Value&);
        std::string ue_ctx_request_handle_security_mode_complete(Json::Value&);
        std::string ue_ctx_request_handle_location_update(Json::Value&);
		std::string ue_ctx_request_handle_create_session(Json::Value&);
		std::string ue_ctx_update_handle_craete_session(Json::Value&);
		std::string ue_ctx_request_handle_attach_complete(Json::Value&);
		std::string ue_ctx_update_handle_attach_complete(Json::Value&);
		std::string ue_ctx_request_handle_modify_bearer(Json::Value&);
		std::string ue_ctx_request_handle_detach(Json::Value&);
		std::string ue_ctx_update_set_upf_info(Json::Value&);
		std::string ue_ctx_request_smf_handle_create_session(Json::Value&);
		std::string ue_ctx_update_smf_handle_create_session(Json::Value&);
		std::string ue_ctx_request_smf_handle_modify_bearer(Json::Value&);
		std::string ue_ctx_request_smf_handle_detach(Json::Value&);
        ~Udm();
};

#endif