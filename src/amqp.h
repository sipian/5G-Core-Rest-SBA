#ifndef AMQP_H
#define AMQP_H


#include "diameter.h"
#include "gtp.h"
#include "network.h"
#include "packet.h"
#include "s1ap.h"
#include "sctp_client.h"
#include "sctp_server.h"
#include "security.h"
#include "sync.h"
//#include "amqp.h"
#include "telecom.h"
#include "udp_client.h"
#include "udp_server.h"
#include "utils.h"
extern string amqp_ip;
extern int amqp_port;
class Amqp{
	public:
//		Amqp();
        	UdpServer amqp_server;
		void pubsub(Packet pkt);
//		~Amqp();
};




#endif
