#include "smf_server.h"
#include <nghttp2/asio_http2_server.h>
#include <jsoncpp/json/json.h>

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;

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
	for (i = 0; i < g_s11_server_threads_count; i++) {
		g_s11_server_threads[i] = thread(handle_s11_traffic, i);
	}	

	/* Joining all threads */
	for (i = 0; i < g_s11_server_threads_count; i++) {
		if (g_s11_server_threads[i].joinable()) {
			g_s11_server_threads[i].join();
		}
	}				
}

void createSMContextRequestPayload(CreateSMContextRequestPacket& request, Json::Value &root) {
	istringstream iss;
	string pktField_string;

	if(root.isMember("guti")) {
		iss.clear();
		pktField_string = root["guti"].asString();
		iss.str(pktField_string);
		iss >> request.guti;
	}

	if(root.isMember("imsi")) {
		iss.clear();
		pktField_string = root["imsi"].asString();
		iss.str(pktField_string);
		iss >> request.imsi;
	}

	if(root.isMember("s11_cteid_mme")) {
		iss.clear();
		pktField_string = root["s11_cteid_mme"].asString();
		iss.str(pktField_string);
		iss >> request.s11_cteid_mme;
	}

	if(root.isMember("eps_bearer_id")) {
		iss.clear();
		pktField_string = root["eps_bearer_id"].asString();
		iss.str(pktField_string);
		iss >> request.eps_bearer_id;
	}

	if(root.isMember("apn_in_use")) {
		iss.clear();
		pktField_string = root["apn_in_use"].asString();
		iss.str(pktField_string);
		iss >> request.apn_in_use;
	}

	if(root.isMember("tai")) {
		iss.clear();
		pktField_string = root["tai"].asString();
		iss.str(pktField_string);
		iss >> request.tai;
	}
}

void createSMContextResponsePayload(CreateSMContextResponsPacket &responsePkt, Json::Value &json_response) {
	json_response["guti"] = to_string(responsePkt.guti);
	json_response["eps_bearer_id"] = to_string(responsePkt.eps_bearer_id);
	json_response["e_rab_id"] = to_string(responsePkt.e_rab_id);
	json_response["s1_uteid_ul"] = to_string(responsePkt.s1_uteid_ul);
	json_response["s11_cteid_sgw"] = to_string(responsePkt.s11_cteid_sgw);
	json_response["k_enodeb"] = to_string(responsePkt.k_enodeb);
	json_response["tai_list_size"] = to_string(responsePkt.tai_list_size);
	json_response["tai_list"] = Json::arrayValue;
	for(int i = 0; i < responsePkt.tai_list_size; i++) {
		json_response["tai_list"].append(to_string(responsePkt.tai_list[i]));
	}
	json_response["tau_timer"] = to_string(responsePkt.tau_timer);
	json_response["ue_ip_addr"] = responsePkt.ue_ip_addr;
	json_response["upf_s1_ip_addr"] = responsePkt.upf_s1_ip_addr;
	json_response["upf_s1_port"] = to_string(responsePkt.upf_s1_port);
	json_response["res"] = to_string(responsePkt.res);
}

void updateSMContextRequestPayload(UpdateSMContextRequestPacket& request, Json::Value &root) {
	istringstream iss;
	string pktField_string;

	if(root.isMember("guti")) {
		iss.clear();
		pktField_string = root["guti"].asString();
		iss.str(pktField_string);
		iss >> request.guti;
	}

	if(root.isMember("s1_uteid_dl")) {
		iss.clear();
		pktField_string = root["s1_uteid_dl"].asString();
		iss.str(pktField_string);
		iss >> request.s1_uteid_dl;
	}

	if(root.isMember("eps_bearer_id")) {
		iss.clear();
		pktField_string = root["eps_bearer_id"].asString();
		iss.str(pktField_string);
		iss >> request.eps_bearer_id;
	}

	if(root.isMember("g_trafmon_ip_addr")) {
		iss.clear();
		pktField_string = root["g_trafmon_ip_addr"].asString();
		iss.str(pktField_string);
		iss >> request.g_trafmon_ip_addr;
	}

	if(root.isMember("g_trafmon_port")) {
		iss.clear();
		pktField_string = root["g_trafmon_port"].asString();
		iss.str(pktField_string);
		iss >> request.g_trafmon_port;
	}
}

void updateSMContextResponsePayload(UpdateSMContextResponsePacket &responsePkt, Json::Value &json_response) {
	json_response["res"] = to_string(responsePkt.res);
}


void releaseSMContextRequestPayload(ReleaseSMContextRequestPacket& request, Json::Value &root) {
	istringstream iss;
	string pktField_string;

	if(root.isMember("guti")) {
		iss.clear();
		pktField_string = root["guti"].asString();
		iss.str(pktField_string);
		iss >> request.guti;
	}

	if(root.isMember("eps_bearer_id")) {
		iss.clear();
		pktField_string = root["eps_bearer_id"].asString();
		iss.str(pktField_string);
		iss >> request.eps_bearer_id;
	}

	if(root.isMember("tai")) {
		iss.clear();
		pktField_string = root["tai"].asString();
		iss.str(pktField_string);
		iss >> request.tai;
	}
}

void releaseSMContextResponsePayload(ReleaseSMContextResponsePacket &responsePkt, Json::Value &json_response) {
	json_response["res"] = to_string(responsePkt.res);
}

void handle_s11_traffic(int worker_id) {
	UdpClient upf_client;
	SctpClient udm_client;

	string smf_ausf_port = to_string(SMF_AMF_PORT_START_RANGE + worker_id);
	upf_client.set_client(smf_upf_ip_addr);
	udm_client.conn(g_udm_ip_addr, g_udm_port);

	http2 server;
	try {

			server.handle("/Nsmf_PDUSession/CreateSMContext", [&worker_id, &upf_client, &udm_client](const request &req, const response &res) {
				req.on_data([&worker_id, &upf_client, &udm_client, &res](const uint8_t *data, size_t len) {
					if(len <= 0) {
						return;
					}
					const char *s = reinterpret_cast<const char *>(data);

					string str;
					str.assign(s,len);
					cout<<"smfserver_handle_s11_traffic :: Nsmf_PDUSession :: CreateSMContext " << str << " is being received" << endl;

					Json::Value root;
					Json::Reader reader;
					bool parsingSuccessful = reader.parse(str,root);

					if(parsingSuccessful == false) {
						res.write_head(400);
						res.end();
						cout << "smfserver_handle_s11_traffic :: Nsmf_PDUSession :: CreateSMContext :: JSON Parsing unsuccessful" << endl;
						return;
					}

					CreateSMContextRequestPacket requestPkt;
					CreateSMContextResponsPacket responsePkt;
					createSMContextRequestPayload(requestPkt, root);
					g_smf.handle_create_session(requestPkt, responsePkt, upf_client, worker_id);
					Json::Value json_response;
					createSMContextResponsePayload(responsePkt, json_response);
					Json::FastWriter fastWriter;
					string jsonMessage = fastWriter.write(json_response);
					res.write_head(200);
					res.end(jsonMessage);

					cout << "smfserver_handle_s11_traffic :: Nsmf_PDUSession :: CreateSMContext :: Attach Response \n\t" << jsonMessage << endl;		
				});
			});

			server.handle("/Nsmf_PDUSession/UpdateSMContext", [&worker_id, &upf_client, &udm_client](const request &req, const response &res) {

				req.on_data([&worker_id, &upf_client, &udm_client, &res](const uint8_t *data, size_t len) {
					if(len <= 0) {
						return;
					}
					const char *s = reinterpret_cast<const char *>(data);

					string str;
					str.assign(s,len);
					cout<<"smfserver_handle_s11_traffic :: Nsmf_PDUSession :: UpdateSMContext " << str << " is being received" << endl;

					Json::Value root;
					Json::Reader reader;
					bool parsingSuccessful = reader.parse(str,root);

					if(parsingSuccessful == false) {
						res.write_head(400);
						res.end();
						cout << "smfserver_handle_s11_traffic :: Nsmf_PDUSession :: UpdateSMContext :: JSON Parsing unsuccessful" << endl;
						return;
					}

					UpdateSMContextRequestPacket requestPkt;
					UpdateSMContextResponsePacket responsePkt;
					updateSMContextRequestPayload(requestPkt, root);
					g_smf.handle_modify_bearer(requestPkt, responsePkt, upf_client, worker_id);
					Json::Value json_response;
					updateSMContextResponsePayload(responsePkt, json_response);
					Json::FastWriter fastWriter;
					string jsonMessage = fastWriter.write(json_response);
					res.write_head(200);
					res.end(jsonMessage);
					cout << "smfserver_handle_s11_traffic :: Nsmf_PDUSession :: UpdateSMContext :: Modify Bearer Response \n\t" << jsonMessage << endl;		
				});
			});

			server.handle("/Nsmf_PDUSession/ReleaseSMContext", [&worker_id, &upf_client, &udm_client](const request &req, const response &res) {

				req.on_data([&worker_id, &upf_client, &udm_client, &res](const uint8_t *data, size_t len) {
					if(len <= 0) {
						return;
					}
					const char *s = reinterpret_cast<const char *>(data);

					string str;
					str.assign(s,len);
					cout<<"smfserver_handle_s11_traffic :: Nsmf_PDUSession :: ReleaseSMContext " << str << " is being received" << endl;

					Json::Value root;
					Json::Reader reader;
					bool parsingSuccessful = reader.parse(str,root);

					if(parsingSuccessful == false) {
						res.write_head(400);
						res.end();
						cout << "smfserver_handle_s11_traffic :: Nsmf_PDUSession :: ReleaseSMContext :: JSON Parsing unsuccessful" << endl;
						return;
					}

					ReleaseSMContextRequestPacket requestPkt;
					ReleaseSMContextResponsePacket responsePkt;
					releaseSMContextRequestPayload(requestPkt, root);
					g_smf.handle_detach(requestPkt, responsePkt, upf_client, worker_id);
					Json::Value json_response;
					releaseSMContextResponsePayload(responsePkt, json_response);
					Json::FastWriter fastWriter;
					string jsonMessage = fastWriter.write(json_response);
					res.write_head(200);
					res.end(jsonMessage);
					cout << "smfserver_handle_s11_traffic :: Nsmf_PDUSession :: ReleaseSMContext :: Detach Response \n\t" << jsonMessage << endl;		
				});
			});

			boost::system::error_code ec;
			if (server.listen_and_serve(ec, smf_amf_ip_addr, smf_ausf_port))
			{
				cerr << "error: " << ec.message() << endl;
			}
	}
	catch (exception &e)
	{
		cerr << "AMF-SMF server :: exception :: " << e.what() << "\n";
	}
}

int main(int argc, char *argv[]) {
	check_usage(argc);
	init(argv);
	run();
	return 0;
}
