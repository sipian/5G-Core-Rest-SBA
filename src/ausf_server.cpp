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
	string port = to_string(AUSF_AMF_PORT_START_RANGE + worker_id);
	try
	{
		server.handle("/Nausf_UEAuthentication", [&worker_id](const request &req, const response &res) 
		{
			req.on_data([&worker_id,&res](const uint8_t *data, size_t len)
			{
				if(len <= 0) {
					return;
				}

				TRACE(cout<<"ausfserver_rest_server :: AMF Request is being received"<<endl;)
				const char *s = "";
				s = reinterpret_cast<const char *>(data);
				string str;
				str.assign(s,len);
				TRACE(cout<<"ausfserver_rest_server :: Received Req: "<<str<<endl;)
				Json::Value root;
				Json::Reader reader;
				bool parsingSuccessful = reader.parse(str,root);

				if(!parsingSuccessful) {
					res.write_head(400);
					res.end();
					cout << "ausfserver_rest_server :: JSON Parsing unsuccessful" << endl;
					return;
				}

				TRACE(cout << "ausfserver_rest_server :: " << "Nausf_UEAuthentication" << endl;)		
				res.write_head(200);
				res.end(g_ausf.handle_autninfo_req(root, worker_id));
				cout<<"ausfserver_rest_server :: Sent Response to AMF." <<endl;
			});
		});

		server.handle("/Nausf_UELocationUpdate", [&worker_id](const request &req, const response &res) 
		{
			req.on_data([&worker_id,&res](const uint8_t *data, size_t len)
			{
				if(len <= 0) {
					return;
				}

				TRACE(cout<<"ausfserver_rest_server :: AMF Request is being received"<<endl;)
				const char *s = "";
				s = reinterpret_cast<const char *>(data);
				string str;
				str.assign(s,len);
				TRACE(cout<<"ausfserver_rest_server :: Received Req: "<<str<<endl;)
				Json::Value root;
				Json::Reader reader;
				bool parsingSuccessful = reader.parse(str,root);

				if(!parsingSuccessful) {
					res.write_head(400);
					res.end();
					cout << "ausfserver_rest_server :: JSON Parsing unsuccessful" << endl;
					return;
				}

				TRACE(cout << "ausfserver_rest_server :: " << " case 1: Nausf_UELocationUpdate" << endl;)		
				res.write_head(200);
				res.end(g_ausf.handle_location_update(root, worker_id));
				cout<<"ausfserver_rest_server :: Sent Response to AMF" <<endl;
			});
		});

		// synchronous call to server.
		if (server.listen_and_serve(ec,g_ausf_ip_addr, port)) 
		{
			cerr << "error: " << ec.message() << endl;
		}
	// }
	}
	catch (exception &e)
	{
		cerr << "AUSF server :: exception :: " << e.what() << "\n";
	}
}

void run() {
	/* MySQL connection */
	// g_ausf.handle_mysql_conn();
	/* HSS server */
	
	for(int i=0;i<g_workers_count;i++)
		rest_servers[i] = thread(rest_server,i);
	
	TRACE(cout<<"REST Server started" << endl;)
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
