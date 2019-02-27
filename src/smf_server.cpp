#include "smf_server.h"

int g_s11_server_threads_count;
vector<thread> g_s11_server_threads;
Smf g_smf;

void check_usage(int argc) {
	if (argc < 2) {
		TRACE(cout << "Usage: ./<smf_server_exec> S11_SERVER_THREADS_COUNT S1_SERVER_THREADS_COUNT" << endl;)
		g_utils.handle_type1_error(-1, "Invalid usage error: smfserver_checkusage");
	}
}

void init(char *argv[]) {
	g_s11_server_threads_count = atoi(argv[1]);
	g_s11_server_threads.resize(g_s11_server_threads_count);
	signal(SIGPIPE, SIG_IGN);
}

void run() {
	int i;

	/* SGW S11 server */
	TRACE(cout << "SMF server started" << endl;)
	g_smf.amf_server.run(smf_amf_ip_addr, smf_amf_port);
	for (i = 0; i < g_s11_server_threads_count; i++) {
		g_s11_server_threads[i] = thread(handle_s11_traffic);
	}	

	/* Joining all threads */
	for (i = 0; i < g_s11_server_threads_count; i++) {
		if (g_s11_server_threads[i].joinable()) {
			g_s11_server_threads[i].join();
		}
	}				
}

void handle_s11_traffic() {
	UdpClient upf_client;
	SctpClient udm_client;
	struct sockaddr_in src_sock_addr;
	Packet pkt;

	upf_client.set_client(smf_upf_ip_addr);
	udm_client.conn(g_udm_ip_addr, g_udm_port);

	while (1) {
		g_smf.amf_server.rcv(src_sock_addr, pkt);
		pkt.extract_gtp_hdr();
		switch(pkt.gtp_hdr.msg_type) {
			/* Create session */
			case 1:
				TRACE(cout << "smfserver_handles11traffic:" << " case 1: create session" << endl;)
				g_smf.handle_create_session(src_sock_addr, pkt, upf_client, udm_client);
				break;

			/* Modify bearer */
			case 2:
				TRACE(cout << "smfserver_handles11traffic:" << " case 2: modify bearer" << endl;)
				g_smf.handle_modify_bearer(src_sock_addr,pkt, upf_client, udm_client);
				break;

			/* Detach */
			case 3:
				TRACE(cout << "smfserver_handles11traffic:" << " case 3: detach" << endl;)
				g_smf.handle_detach(src_sock_addr, pkt, upf_client, udm_client);
				break;
			/* For error handling */
			default:
				TRACE(cout << "smfserver_handles11traffic:" << " default case:" << endl;)
		}		
	}
}

int main(int argc, char *argv[]) {
	check_usage(argc);
	init(argv);
	run();
	return 0;
}
