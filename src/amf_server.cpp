#include "amf_server.h"

Amf g_amf;
int g_workers_count;
vector<SctpClient> ausf_clients;
vector<UdpClient> smf_clients;
vector<SctpClient> udm_clients;

void check_usage(int argc) {
	if (argc < 2) {
		TRACE(cout << "Usage: ./<amf_server_exec> THREADS_COUNT" << endl;)
		g_utils.handle_type1_error(-1, "Invalid usage error: amfserver_checkusage");
	}
}

void init(char *argv[]) {
	g_workers_count = atoi(argv[1]);
	ausf_clients.resize(g_workers_count);
	smf_clients.resize(g_workers_count);
	udm_clients.resize(g_workers_count);
	signal(SIGPIPE, SIG_IGN);
}

void run() {
	int i;
	
	TRACE(cout << "AMF server started" << endl;)
	
	for (i = 0; i < g_workers_count; i++) {
		ausf_clients[i].conn(g_ausf_ip_addr, g_ausf_port);	
		smf_clients[i].conn(g_amf_ip_addr, smf_amf_ip_addr, smf_amf_port);
		udm_clients[i].conn(g_udm_ip_addr, g_udm_port);
	}
	
	g_amf.server.run(g_amf_ip_addr, g_amf_port, g_workers_count, handle_ue);
}

int handle_ue(int conn_fd, int worker_id) {
	bool res;
	Packet pkt;

	g_amf.server.rcv(conn_fd, pkt);
	if (pkt.len <= 0) {
		TRACE(cout << "amfserver_handleue:" << " Connection closed" << endl;)
		return 0;
	}
	pkt.extract_s1ap_hdr();
	if (pkt.s1ap_hdr.mme_s1ap_ue_id == 0) {
		switch (pkt.s1ap_hdr.msg_type) {
			/* Initial Attach request */
			case 1: 
				TRACE(cout << "amfserver_handleue:" << " case 1: initial attach" << endl;)
				g_amf.handle_initial_attach(conn_fd, pkt, ausf_clients[worker_id], worker_id);
				break;

			/* For error handling */
			default:
				TRACE(cout << "amfserver_handleue:" << " default case: new" << endl;)
				break;
		}		
	}
	else if (pkt.s1ap_hdr.mme_s1ap_ue_id > 0) {
		switch (pkt.s1ap_hdr.msg_type) {
			/* Authentication response */
			case 2: 
				TRACE(cout << "amfserver_handleue:" << " case 2: authentication response" << endl;)
				res = g_amf.handle_autn(conn_fd, pkt, udm_clients[worker_id]);
				if (res) {
					g_amf.handle_security_mode_cmd(conn_fd, pkt, udm_clients[worker_id]);
				}
				break;

			/* Security Mode Complete */
			case 3: 
				TRACE(cout << "amfserver_handleue:" << " case 3: security mode complete" << endl;)
				res = g_amf.handle_security_mode_complete(conn_fd, pkt, udm_clients[worker_id]);
				if (res) {
				        g_amf.handle_location_update(pkt, ausf_clients[worker_id], udm_clients[worker_id]);
						g_amf.handle_create_session(conn_fd, pkt, smf_clients[worker_id], udm_clients[worker_id], worker_id);
				}
				break;

			/* Attach Complete */
			case 4: 
				TRACE(cout << "amfserver_handleue:" << " case 4: attach complete" << endl;)
				g_amf.handle_attach_complete(pkt, udm_clients[worker_id]);
				g_amf.handle_modify_bearer(pkt, smf_clients[worker_id], udm_clients[worker_id], worker_id);
				break;

			/* Detach request */
			case 5: 
				TRACE(cout << "amfserver_handleue:" << " case 5: detach request" << endl;)
				g_amf.handle_detach(conn_fd, pkt, smf_clients[worker_id], udm_clients[worker_id], worker_id);
				break;
			case 7:
				cout << "amfserver_handleue:" << " case 7:" << endl;
				g_amf.handle_handover(pkt);

				break;
			case 8:
				cout << "amfserver_handleue:" << " case 8:" << endl;
				g_amf.setup_indirect_tunnel(pkt);

				break;
			case 9:
				cout << "amfserver_handleue:" << " case 9:" << endl;
				g_amf.handle_handover_completion(pkt);

				break;
			case 10:
				cout << "send indirect tunnel teardwn req:" << " case 10:" << endl;
				g_amf.teardown_indirect_tunnel(pkt);
			/* For error handling */	
			default:
				TRACE(cout << "amfserver_handleue:" << " default case: attached" << endl;)
				break;
		}				
	}		
	return 1;
}

int main(int argc, char *argv[]) {
	check_usage(argc);
	void pub_sub();
	init(argv);
	run();
	return 0;
}
