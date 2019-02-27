#ifndef SMF_SERVER_H
#define SMF_SERVER_H

#include "diameter.h"
#include "gtp.h"
#include "network.h"
#include "packet.h"
#include "s1ap.h"
#include "smf.h"
#include "sync.h"
#include "udp_client.h"
#include "udp_server.h"
#include "utils.h"

extern int g_s11_server_threads_count;
extern vector<thread> g_s11_server_threads;
extern Smf g_smf;

void check_usage(int);
void init(char**);
void run();
void handle_s11_traffic();

#endif /* SGW_SERVER_H */
