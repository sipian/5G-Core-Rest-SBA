#ifndef UDM_SERVER_H
#define UDM_SERVER_H

#include "diameter.h"
#include "gtp.h"
#include "udm.h"
#include "mysql.h"
#include "network.h"
#include "packet.h"
#include "s1ap.h"
#include "sctp_server.h"
#include "sync.h"
#include "utils.h"

extern Udm g_udm;
extern int g_workers_count;

void check_usage(int);
void init(char**);
void run();
void handle_udm(int);
void finish();

#endif