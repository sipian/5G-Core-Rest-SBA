#include "amqp.h"
string amqp_ip="10.0.3.183";
int amqp_port=8234;


void Amqp:: pubsub(Packet pkt)
{
	string entity_name;
	uint8_t type;
	//string service_name; 
	uint64_t size;
	vector<uint64_t> service_list;
	pkt.extract_item(entity_name);
	pkt.extract_item(type);
	pkt.extract_item(size);
	pkt.extract_item(service_list,size);
	TRACE(cout << "entity name=" <<entity_name;)
}
