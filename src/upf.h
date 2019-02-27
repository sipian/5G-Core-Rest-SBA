#ifndef UPF_H
#define UPF_H

#include "diameter.h"
#include "gtp.h"
#include "network.h"
#include "packet.h"
#include "s1ap.h"
#include "sync.h"
#include "udp_client.h"
#include "udp_server.h"
#include "utils.h"


extern string g_upf_s11_ip_addr;
extern string g_upf_s1_ip_addr;
//extern string g_sgw_s5_ip_addr;
extern int g_upf_s11_port;
extern int g_upf_s1_port;
//extern int g_sgw_s5_port;

extern string g_upf_sgi_ip_addr;
extern string g_sink_ip_addr;
extern int g_upf_sgi_port;
extern int g_sink_port;


class UeContext {
public:

        /* UE location info */
        string ip_addr;
        uint64_t tai; /* Tracking Area Identifier */

        /* EPS session info */
        uint64_t apn_in_use; /* Access Point Name in Use */

        /* EPS Bearer info */
        uint8_t eps_bearer_id; /* Evolved Packet System Bearer Id */
        uint32_t s1_uteid_ul; /* S1 Userplane Tunnel Endpoint Identifier - Uplink */
        uint32_t s1_uteid_dl; /* S1 Userplane Tunnel Endpoint Identifier - Downlink */
	  uint32_t s11_cteid_smf; /* S11 Controlplane Tunnel Endpoint Identifier - MME */
        uint32_t s11_cteid_upf; /* S11 Controlplane Tunnel Endpoint Identifier - SGW */
      uint32_t s1_cteid_ul; /* S5 Controlplane Tunnel Endpoint Identifier - Uplink */
      uint32_t s1_cteid_dl; /* S5 Controlplane Tunnel Endpoint Identifier - Downlink */

        /* PGW info */
//      string pgw_s5_ip_addr;
//      int pgw_s5_port;

        /* eNodeB info */
        string enodeb_ip_addr;
        int enodeb_port;
	uint32_t sgi_cteid_ul;
        UeContext();
        void init(string,uint64_t, uint64_t, uint8_t, uint32_t, uint32_t,uint32_t);
        ~UeContext();
};

class Upf {
private:
        unordered_map<uint32_t, uint64_t> s11_id; /* S11 UE identification table: s11_cteid_sgw -> imsi */
        unordered_map<uint32_t, uint64_t> s1_id; /* S1 UE identification table: s1_uteid_ul -> imsi */
        unordered_map<string, uint64_t> sgi_id; /* S5 UE identification table: s5_uteid_dl -> imsi */
        unordered_map<uint64_t, UeContext> ue_ctx; /* UE context table: imsi -> UeContext */

 unordered_map<uint32_t, UeContext> ho_ue_ctx; /* UE context table: imsi -> UeContext */

unordered_map<uint64_t, string> ip_addrs;
        /* Lock parameters */
        pthread_mutex_t s11id_mux; /* Handles s11_id */
        pthread_mutex_t s1id_mux; /* Handles s1_id */
        pthread_mutex_t sgiid_mux; /* Handles s5_id */
        pthread_mutex_t uectx_mux; /* Handles ue_ctx */

        void clrstl();
        void set_ip_addrs();
        void update_itfid(int, uint32_t, uint64_t);
        void update_itfid(int, uint32_t, string, uint64_t);
        uint64_t get_imsi(int, uint32_t);
	uint64_t get_imsi(int, uint32_t,string);

        bool get_uplink_info(uint64_t, uint32_t&, string&, int&);
        bool get_downlink_info(uint64_t, uint32_t&, string&, int&);
        void rem_itfid(int, uint32_t);
        void rem_uectx(uint64_t);
        //void set_ip_addrs();
        void rem_itfid(int, uint32_t, string);
public:
        UdpServer s11_server;
        UdpServer s1_server;
        //UdpServer s5_server;
        UdpServer sgi_server;
        Upf();
        void handle_create_session(struct sockaddr_in, Packet);
       ///void handle_create_session(struct sockaddr_in, Packet);
        void handle_modify_bearer(struct sockaddr_in, Packet);
        void handle_uplink_udata(Packet, UdpClient&);
        void handle_downlink_udata(Packet, UdpClient&);
        void handle_detach(struct sockaddr_in, Packet);
        //handover changes
        void handle_indirect_tunnel_setup(struct sockaddr_in src_sock_addr, Packet pkt);
        void handle_handover_completion(struct sockaddr_in src_sock_addr,Packet pkt);
        void handle_indirect_tunnel_teardown_(struct sockaddr_in src_sock_addr,Packet pkt);
        //

        ~Upf();
};

#endif /* SGW_H */


 
