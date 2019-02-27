#include "ausf_server.h"
#include <iostream>
#include <bits/stdc++.h>
#include <nghttp2/asio_http2_server.h>
#include <jsoncpp/json/json.h>
#include <sstream>

using namespace std;
using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;

Ausf g_ausf;
int g_workers_count;
vector<SctpClient> udm_clients;
vector<thread> rest_servers;
Json::FastWriter fastWriter;

void check_usage(int argc) {
	if (argc < 2) {
		TRACE(cout << "Usage: ./<ausf_server_exec> THREADS_COUNT" << endl;)
		g_utils.handle_type1_error(-1, "Invalid usage error: ausfserver_checkusage");
	}
}

void init(char *argv[]) {
	g_workers_count = atoi(argv[1]);
	udm_clients.resize(g_workers_count);
	rest_servers.resize(g_workers_count);
	if (mysql_library_init(0, NULL, NULL)) {
		g_utils.handle_type1_error(-1, "mysql_library_init error: ausfserver_init");
	}
	signal(SIGPIPE, SIG_IGN);
}

void rest_server(int worker_id)
{
	boost::system::error_code ec;
  	http2 server;
	string port = "400" + to_string(worker_id);
	RestPacket rpkt;

	while(1)
	{
		server.handle("/Nausf_UEAuthentication", [&worker_id,&rpkt](const request &req, const response &res) 
		{
			boost::system::error_code ec;

			req.on_data([&worker_id,&rpkt,&res](const uint8_t *data, std::size_t len) 
			{

				if(len <= 0)
					 return;

				cout<<"AuSF Request is being received"<<endl;
				const char *s = "";
				s = reinterpret_cast<const char *>(data);
				std::string str;
				str.assign(s,len);
				cout<<"Received Req: "<<str<<endl;
				Json::Value root;
				Json::Reader reader;
				bool parsingSuccessful = reader.parse(str,root);
				Packet pkt;
				pkt.clear_pkt();
		
				uint64_t imsi;
				uint64_t plmn_id;
				uint64_t autn_vector;
				uint64_t nw_type;
				uint64_t autn_num;
				uint64_t rand_num;
				uint64_t xres;
				uint64_t k_asme;


				std::string imsi_string = root["imsi"].asString();
				std::istringstream iss(imsi_string);
				iss >> imsi;

				iss.clear();
				std::string plmn_id_string = root["plmn_id"].asString();
				iss.str(plmn_id_string);
				iss >> plmn_id;
		
				iss.clear();
				std::string autn_vector_string = root["num_autn_vectors"].asString();
				iss.str(autn_vector_string);
				iss >> autn_vector;
		
				iss.clear();
				std::string nw_type_string = root["nw_type"].asString();
				iss.str(nw_type_string);
				iss >> nw_type;

				
				rpkt.imsi = imsi;
				rpkt.plmn_id = plmn_id;
				rpkt.autn_vector = autn_vector;
				rpkt.nw_type = nw_type;

				TRACE(cout << "ausfserver_handlemme:" << " case 1: autn info req" << endl;)
				g_ausf.handle_autninfo_req(rpkt, udm_clients[worker_id]);
				std::cout<<"rpkt.autn_num is (ausf_server.cpp) "<<rpkt.autn_num<<endl;
				autn_num = rpkt.autn_num;
				rand_num = rpkt.rand_num;
				xres = rpkt.xres;
				k_asme = rpkt.k_asme;
				//make the reply here 
				Json::Value json_response;

				std:cout<<"autn_num"<<autn_num<<endl;


				json_response["autn_num"] = to_string(autn_num);
				json_response["rand_num"] = to_string(rand_num);
				json_response["xres"] = to_string(xres);
				json_response["k_asme"] = to_string(k_asme);
				Json::FastWriter fastWriter;
		
				std::string jsonMessage = fastWriter.write(json_response);
		
				res.write_head(200);
				res.end(jsonMessage);
				cout<<"Response Sent"<<endl;

			});
		});


		if (server.listen_and_serve(ec,g_ausf_ip_addr, port)) 
		{
			std::cerr<<"hjdhfjdhfdjsfhs"<<std::endl;
			std::cerr << "error: " << ec.message() << std::endl;
		}
	// }
	}
}

void run() {
	/* MySQL connection */
	// g_ausf.handle_mysql_conn();
	/* HSS server */
	
	for(int i=0;i<g_workers_count;i++){
		udm_clients[i].conn(g_udm_ip_addr, g_udm_port);
	}
	TRACE(cout << "udm clients started" << endl;)

	for(int i=0;i<g_workers_count;i++)
		rest_servers[i] = thread(rest_server,i);
	
	TRACE(cout<<"REST Server started" << endl;)

	TRACE(cout << "Ausf server started" << endl;)
	g_ausf.server.run(g_ausf_ip_addr, g_ausf_port, g_workers_count, handle_mme);

}


int handle_mme(int conn_fd, int worker_id) {
	Packet pkt;

	g_ausf.server.rcv(conn_fd, pkt);
	if (pkt.len <= 0) {
		TRACE(cout << "ausfserver_handlemme:" << " Connection closed" << endl;)
		return 0;
	}		
	pkt.extract_diameter_hdr();
	switch (pkt.diameter_hdr.msg_type) {
		/* Authentication info req */

		/* Location update */
		case 2:
			TRACE(cout << "ausfserver_handlemme:" << " case 2: loc update" << endl;)
			g_ausf.handle_location_update(conn_fd, pkt, udm_clients[worker_id]);
			break;

		/* For error handling */	
		default:
			TRACE(cout << "ausfserver_handlemme:" << " default case:" << endl;)
			break;
	}
	return 1;
}

void finish() {
	mysql_library_end();		
}

int main(int argc, char *argv[]) {
	check_usage(argc);
	init(argv);
	run();
	finish();
	return 0;
}
