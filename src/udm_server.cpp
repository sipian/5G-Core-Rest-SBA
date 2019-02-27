#include "udm_server.h"
int g_workers_count;
Udm g_udm;

void check_usage(int argc)
{
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

void run()
{
    g_udm.handle_mysql_conn();
    TRACE(cout<<"Udm server started"<<endl;)
    g_udm.server.run(g_udm_ip_addr, g_udm_port, g_workers_count, handle_udm);
}

int handle_udm(int conn_fd, int worker_id)
{
    Packet pkt;
    TRACE(cout<<"udm thread waiting for the packet"<<endl;)
    g_udm.server.rcv(conn_fd, pkt);
    if(pkt.len <= 0 )
    {
        TRACE(cout<<"udmserver_handleudm: "<<"connection_closed"<<endl;)
        return 0;
    }
    pkt.extract_diameter_hdr();
    switch (pkt.diameter_hdr.msg_type)
    {
        case 1: 
            TRACE(cout<<"udm server handle_udm: "<<"case: 1 handle_autn_info"<< endl;)
            g_udm.get_autn_info(conn_fd, pkt);
            break;
        case 2:
            TRACE(cout<<"udm server handle_udm: "<<"case: 2 handle_loc_update"<< endl;)
            g_udm.set_loc_info(conn_fd, pkt);
            break;

        case 3:
            TRACE(cout<<"udm_server handle_ue_updation"<<"case: 3 update from amf_handle_initial_attach_init"<<endl;)
            g_udm.update_info_amf_initial_attach_init(conn_fd, pkt);
            break;

        case 4:
            TRACE(cout<<"udm_server handle_ue_updation"<<"case: 4 update from amf_handle_initial_attach"<<endl;)
            g_udm.update_info_amf_initial_attach(conn_fd, pkt);
            break;
        
        case 5:
            TRACE(cout<<"udm server handle autn"<<"case: 5 ue ctx request from amf handle autn"<<endl;)
            g_udm.handle_autn_ue_ctx_request(conn_fd, pkt);
            break;

        case 6:
            TRACE(cout<<"udm_server ue_ctx requests"<<"case: 6 ue_ctx request from AMF_security mode cmd"<<endl;)
            g_udm.ue_ctx_request_security_mode_cmd(conn_fd, pkt);
            break;

        case 7:
            TRACE(cout<<"udm server set encrypt context"<<"case: 7 ue ctx information for k_nas_enc"<<endl;)
            g_udm.ue_ctx_request_set_crypt_context(conn_fd, pkt);
            break;
        
        case 8:
            TRACE(cout<<"udm server set integrity context"<<endl;)
            g_udm.ue_ctx_update_set_integrity_context(conn_fd, pkt);
            break;


        case 9:
            TRACE(cout<<"udm server set encrypt context"<<"Case: 9 ue ctx update"<<endl;)
            g_udm.ue_ctx_request_handle_security_mode_complete(conn_fd, pkt);
            break;

        case 10:
            TRACE(cout<<"udm server handle location update"<<"Case: 10 ue ctx request from location update"<<endl;)
            g_udm.ue_ctx_request_handle_location_update(conn_fd, pkt);
            break;

        case 11:
            TRACE(cout<<"udm server request handle create session"<<"Case: 11 ue ctx update request for create session"<<endl;)
            g_udm.ue_ctx_request_handle_create_session(conn_fd, pkt);
            break; 

        case 12:
            TRACE(cout<<"udm server update handle create session"<<"Case: 12 ue ctx update request for create session"<<endl;)
            g_udm.ue_ctx_update_handle_craete_session(conn_fd, pkt);
            break;

        case 13:
            TRACE(cout<<"udm server request handle attach complete"<<"Case 13: "<<endl;)
            g_udm.ue_ctx_request_handle_attach_complete(conn_fd, pkt);
            break;

        case 14:
            TRACE(cout<<"udm server update handle attach complete"<<"Case 14: "<<endl;)
            g_udm.ue_ctx_update_handle_attach_complete(conn_fd, pkt);
            break;

        case 15:
            TRACE(cout<<"udm server request handle modify bearer"<<"Case 15: "<<endl;)
            g_udm.ue_ctx_request_handle_modify_bearer(conn_fd, pkt);
            break;  

        case 16:
            TRACE(cout<<"udm server request from handle detach"<<"Case 16: "<<endl;)
            g_udm.ue_ctx_request_handle_detach(conn_fd, pkt);
            break;

        case 17:
            TRACE(cout<<"udm server update from set upf info"<<"Case 17: "<<endl;)
            g_udm.ue_ctx_update_set_upf_info(conn_fd, pkt);
            break;
        
        case 18:
            TRACE(cout<<"udm server request from SMF handle create session"<<"Case 18: "<<endl;)
            g_udm.ue_ctx_request_smf_handle_create_session(conn_fd, pkt);
            break;

        case 19:
            TRACE(cout<<"udm server request from SMF handle create session"<<"Case 19: "<<endl;)
            g_udm.ue_ctx_update_smf_handle_create_session(conn_fd, pkt);
            break;
        
        case 20:
            TRACE(cout<<"udm server request from SMF handle modify bearer"<<"Case 20: "<<endl;)
            g_udm.ue_ctx_request_smf_handle_modify_bearer(conn_fd, pkt);
            break;

        case 21:
            TRACE(cout<<"udm server request from smf handle detach"<<"Case 21: "<<endl;)
            g_udm.ue_ctx_request_smf_handle_detach(conn_fd, pkt);
            break;

        default:
            TRACE(cout<<"udm_server_handle_udm: " << " default case:" << endl;)
            break;
    }
    return 1;

}

void finish()
{
    mysql_library_end();
}

int main(int argc, char *argv[])
{
    check_usage(argc);
    init(argv);
    run();
    finish();
    return 0;
}