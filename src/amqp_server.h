#ifndef AMQP_SERVER_H
#define AMQP_SERVER_H

#include "diameter.h"
#include "gtp.h"
#include "network.h"
#include "packet.h"
#include "s1ap.h"
#include "amqp.h"
#include "sync.h"
#include "udp_client.h"
#include "udp_server.h"
#include "utils.h"

extern Amqp g_amqp;
void handle_traffic();
void run();
void init(char**);
void check_usage(int);
#endif /* SGW_SERVER_H */
