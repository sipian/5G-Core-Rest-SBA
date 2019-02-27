#include "amqp_server.h"
int g_s11_server_threads_count;
vector<thread> g_s11_server_threads;

Amqp g_amqp;
void check_usage(int argc) {
	if (argc < 2) {
		TRACE(cout << "Usage: ./<smf_server_exec> S11_SERVER_THREADS_COUNT S1_SERVER_THREADS_COUNT" << endl;)
		g_utils.handle_type1_error(-1, "Invalid usage error: smfserver_checkusage");
	}
}

void handle_traffic() {
	TRACE(cout << "in handle traffic" ;)
	UdpClient amqp_client;
	struct sockaddr_in src_sock_addr;
	Packet pkt;
//	TRACE(cout << "in handle traffic" ;)
	int case1=1;
	while (1) {
	       TRACE(cout << "in while" ;)
		switch(case1) {
			case 1:
				g_amqp.amqp_server.rcv(src_sock_addr, pkt);
				g_amqp.pubsub(pkt);
				break;
		}
	
	}
	TRACE(cout << "handle traffic ended" ;)
}

void init(char *argv[]) {
	 TRACE(cout << "in init" ;)

	g_s11_server_threads_count = atoi(argv[1]);
	g_s11_server_threads.resize(g_s11_server_threads_count);
	signal(SIGPIPE, SIG_IGN);
}
void run() {
	int i;

	/* SGW S11 server */
	TRACE(cout << "SMF server started" << endl;)
	g_amqp.amqp_server.run(amqp_ip, amqp_port);
	for (i = 0; i < g_s11_server_threads_count; i++) {
		g_s11_server_threads[i] = thread(handle_traffic);
	}	

	/* Joining all threads */
	for (i = 0; i < g_s11_server_threads_count; i++) {
		if (g_s11_server_threads[i].joinable()) {
			g_s11_server_threads[i].join();
		}
	}				
}

int main(int argc, char *argv[]) {
	check_usage(argc);
	init(argv);
	run();
	return 0;
}
