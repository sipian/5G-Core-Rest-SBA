#include "udm_server.h"
#include "ports.h"
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <nghttp2/asio_http2_server.h>
#include <jsoncpp/json/json.h>

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;

int g_workers_count;
Udm g_udm;
std::vector<thread> g_udm_server_threads;

void check_usage(int argc) {
    if(argc < 2)
    {
        cout<<"Invalid usage 2 arguments required"<<endl;
        exit(-1);
    }
}

void init(char *argv[]){
    g_workers_count = atoi(argv[1]);
    if (mysql_library_init(0, NULL, NULL)) {
		g_utils.handle_type1_error(-1, "mysql_library_init error: ausfserver_init");
	}
	signal(SIGPIPE, SIG_IGN);
}

void run() {
    int i;
    g_udm.handle_mysql_conn();

	/* SGW S11 server */
    g_udm_server_threads.resize(g_workers_count);
	for (int i = 0; i < g_workers_count; i++) {
		g_udm_server_threads[i] = std::thread(handle_udm, i);
	}	
    TRACE(cout<<"Udm server started"<<endl;)

	/* Joining all threads */
	for (int i = 0; i < g_workers_count; i++) {
		if (g_udm_server_threads[i].joinable()) {
			g_udm_server_threads[i].join();
		}
	}
}

void append_data(const uint8_t *data, size_t len, std::string &res_string) {
    if (len > 0) {
        const char *s = "";
        s = reinterpret_cast<const char *>(data);
        if(len > 0) {
            res_string.append(s, len);
        }
    }
}

void assign_data(const uint8_t *data, size_t len, std::string &res_string) {
    if (len > 0) {
        const char *s = "";
        s = reinterpret_cast<const char *>(data);
        res_string.assign(s,len);
    }
}

void handle(std::string route, http2 &server, void (*callback)(const response &res, Json::Value &jsonVal)) {
    server.handle(route, [callback, route](const request &req, const response &res) {

        req.on_data([callback, &res, route](const uint8_t *data, size_t len) {
            if(len <= 0) return;
            
            std::string res_string = "";
            assign_data(data, len, res_string);
            
            // TODO: WARNING - CHANGE IT TO CONSUME AFTER ALL CHUNKS ARE RECEIVED.
            TRACE(cout<<"udm :: "<< route <<" :: " << res_string << " is being received" << endl;)
            Json::Value root;
            Json::Reader reader;
            if(!reader.parse(res_string, root)) {
                res.write_head(400);
                res.end();
                TRACE(cout<<"udm :: "<< route <<" :: JSON Parsing unsuccessful" << endl;)
                return;
            }

            callback(res, root);

        });

        // TODO: look for it's replacement. Not available for this server::req object.
        // req.on_close([&res_string](uint32_t error_code) {
        // });
    });
}

void handle_udm(int worker_id) {
	http2 server;
    
    handle("/Nudm_UECM/1", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server handle_udm: "<<"case: 1 handle_autn_info"<< endl;)
        res.write_head(200);
        res.end(g_udm.get_autn_info(jsonVal));
    });

    handle("/Nudm_UECM/2", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server handle_udm: "<<"case: 2 handle_loc_update"<< endl;)
        g_udm.set_loc_info(jsonVal);
        res.write_head(200);
        res.end();
    });

    handle("/Nudm_UECM/3", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm_server handle_ue_updation"<<"case: 3 update from amf_handle_initial_attach_init"<<endl;)
        g_udm.update_info_amf_initial_attach_init(jsonVal);
        res.write_head(200);
        res.end();
    });

    handle("/Nudm_UECM/4", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm_server handle_ue_updation"<<"case: 4 update from amf_handle_initial_attach"<<endl;)
        g_udm.update_info_amf_initial_attach(jsonVal);
        res.write_head(200);
        res.end();
    });
    
    handle("/Nudm_UECM/5", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server handle autn"<<"case: 5 ue ctx request from amf handle autn"<<endl;)
        res.write_head(200);
        res.end(g_udm.handle_autn_ue_ctx_request(jsonVal));
    });

    handle("/Nudm_UECM/6", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm_server ue_ctx requests"<<"case: 6 ue_ctx request from AMF_security mode cmd"<<endl;)
        res.write_head(200);
        res.end(g_udm.ue_ctx_request_security_mode_cmd(jsonVal));
    });

    handle("/Nudm_UECM/7", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server set encrypt context"<<"case: 7 ue ctx information for k_nas_enc"<<endl;)
        g_udm.ue_ctx_request_set_crypt_context(jsonVal);
        res.write_head(200);
        res.end();
    });

    handle("/Nudm_UECM/8", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server set integrity context"<<endl;)
        g_udm.ue_ctx_update_set_integrity_context(jsonVal);
        res.write_head(200);
        res.end();
    });

    handle("/Nudm_UECM/9", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server set encrypt context"<<"Case: 9 ue ctx update"<<endl;)
        res.write_head(200);
        res.end(g_udm.ue_ctx_request_handle_security_mode_complete(jsonVal));
    });

    handle("/Nudm_UECM/10", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server handle location update"<<"Case: 10 ue ctx request from location update"<<endl;)
        res.write_head(200);
        res.end(g_udm.ue_ctx_request_handle_location_update(jsonVal));
    });

    handle("/Nudm_UECM/11", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server request handle create session"<<"Case: 11 ue ctx update request for create session"<<endl;)
        res.write_head(200);
        res.end(g_udm.ue_ctx_request_handle_create_session(jsonVal));
    });

    handle("/Nudm_UECM/12", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server update handle create session"<<"Case: 12 ue ctx update request for create session"<<endl;)
        res.write_head(200);
        res.end(g_udm.ue_ctx_update_handle_craete_session(jsonVal));
    });
    
    handle("/Nudm_UECM/13", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server request handle attach complete"<<"Case 13: "<<endl;)
        res.write_head(200);
        res.end(g_udm.ue_ctx_request_handle_attach_complete(jsonVal));
    });

    handle("/Nudm_UECM/14", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server update handle attach complete"<<"Case 14: "<<endl;)
        g_udm.ue_ctx_update_handle_attach_complete(jsonVal);
        res.write_head(200);
        res.end();
    });

    handle("/Nudm_UECM/15", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server request handle modify bearer"<<"Case 15: "<<endl;)
        res.write_head(200);
        res.end(g_udm.ue_ctx_request_handle_modify_bearer(jsonVal));
    });

    handle("/Nudm_UECM/16", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server request from handle detach"<<"Case 16: "<<endl;)
        res.write_head(200);
        res.end(g_udm.ue_ctx_request_handle_detach(jsonVal));
    });

    handle("/Nudm_UECM/17", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server update from set upf info"<<"Case 17: "<<endl;)
        g_udm.ue_ctx_update_set_upf_info(jsonVal);
        res.write_head(200);
        res.end();
    });

    handle("/Nudm_UECM/18", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server request from SMF handle create session"<<"Case 18: "<<endl;)
        res.write_head(200);
        res.end(g_udm.ue_ctx_request_smf_handle_create_session(jsonVal));
    });

    handle("/Nudm_UECM/19", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server request from SMF handle create session"<<"Case 19: "<<endl;)
        res.write_head(200);
        res.end(g_udm.ue_ctx_update_smf_handle_create_session(jsonVal));
    });

    handle("/Nudm_UECM/20", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server request from SMF handle modify bearer"<<"Case 20: "<<endl;)
        res.write_head(200);
        res.end(g_udm.ue_ctx_request_smf_handle_modify_bearer(jsonVal));
    });

    handle("/Nudm_UECM/21", server, [](const response &res, Json::Value &jsonVal){
        TRACE(cout<<"udm server request from smf handle detach"<<"Case 21: "<<endl;)
        res.write_head(200);
        res.end(g_udm.ue_ctx_request_smf_handle_detach(jsonVal));
    });

	try {
        boost::system::error_code ec;
        if (server.listen_and_serve(ec, g_udm_ip_addr, std::to_string(UDM_PORT_START_RANGE+worker_id))) {
            cerr << "error: " << ec.message() << endl;
        }
	} catch (exception &e) {
		cerr << "UDM server ("<< worker_id <<") :: exception :: " << e.what() << "\n";
	}

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