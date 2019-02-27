#include "sink.h"
#include "discover.h"

// string g_pgw_sgi_ip_addr = "192.168.130.202";
// string g_sink_ip_addr = "192.168.130.162";

string g_upf_ip_addr = resolve_host("upf");
string g_sink_ip_addr = resolve_host("sink");

int g_upf_port = 8100;
int g_sink_port = 8500;

void TrafficMonitor::handle_uplink_udata() {
	struct sockaddr_in src_sock_addr;
	Packet pkt;

	// TRACE(cout << "sink_handleuplinkudata:" << " Data pending from pgw" << endl;)
	server.rcv(src_sock_addr, pkt);
	tun.snd(pkt);
}

void TrafficMonitor::handle_downlink_udata() {
	UdpClient pgw_sgi_client;
	Packet pkt;

	// TRACE(cout << "sink_handledownlinkudata:" << " Data pending at tun" << endl;)
	tun.rcv(pkt);
	pgw_sgi_client.conn(g_sink_ip_addr, g_upf_ip_addr, g_upf_port);
	pgw_sgi_client.snd(pkt);
}
