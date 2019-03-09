#---------------Final Target----------------#

all:	ransim.out sink.out upf.out ausf.out smf.out amf.out udm.out

#-------------------------------------------#


#-----------------Compiler------------------#

G++ = g++ -std=c++0x -std=c++11 -std=gnu++0x -ggdb -pthread -ljsoncpp -lnghttp2_asio -lboost_system -lcrypto -lssl

#-------------------------------------------#


#---------------Object files----------------#

diameter.o:	diameter.cpp diameter.h utils.h
	$(G++) -c -o diameter.o diameter.cpp

gtp.o:	gtp.cpp gtp.h utils.h
	$(G++) -c -o gtp.o gtp.cpp

amf.o: ports.h diameter.h gtp.h network.h packet.h s1ap.h sctp_client.h sctp_server.h security.h sync.h telecom.h udp_client.h utils.h amf.cpp amf.h
	$(G++) -c -o amf.o amf.cpp

amf_server.o: ports.h diameter.h gtp.h amf.h amf_server.cpp amf_server.h network.h packet.h s1ap.h sctp_client.h sctp_server.h security.h sync.h telecom.h udp_client.h utils.h
	$(G++) -c -o amf_server.o amf_server.cpp 

ausf.o: ports.h diameter.h gtp.h ausf.cpp ausf.h mysql.h network.h packet.h s1ap.h sctp_server.h sctp_client.h sync.h utils.h
	$(G++) -c -o ausf.o ausf.cpp      

ausf_server.o: ports.h diameter.h gtp.h ausf.h ausf_server.cpp ausf_server.h mysql.h network.h packet.h s1ap.h sctp_server.h sctp_client.h sync.h utils.h
	$(G++) -c -o ausf_server.o ausf_server.cpp        


hss.o:	diameter.h gtp.h hss.cpp hss.h mysql.h network.h packet.h s1ap.h sctp_server.h sync.h utils.h
	$(G++) -c -o hss.o hss.cpp	

hss_server.o:	diameter.h gtp.h hss.h hss_server.cpp hss_server.h mysql.h network.h packet.h s1ap.h sctp_server.h sync.h utils.h
	$(G++) -c -o hss_server.o hss_server.cpp	

mme.o:	diameter.h gtp.h mme.cpp mme.h network.h packet.h s1ap.h sctp_client.h sctp_server.h security.h sync.h telecom.h udp_client.h utils.h
	$(G++) -c -o mme.o mme.cpp

mme_server.o:	diameter.h gtp.h mme.h mme_server.cpp mme_server.h network.h packet.h s1ap.h sctp_client.h sctp_server.h security.h sync.h telecom.h udp_client.h utils.h
	$(G++) -c -o mme_server.o mme_server.cpp	


smf.o: ports.h diameter.h gtp.h smf.cpp smf.h network.h packet.h s1ap.h sctp_client.h sctp_server.h security.h sync.h telecom.h udp_client.h udp_server.h utils.h
	$(G++) -c -o smf.o smf.cpp

smf_server.o: ports.h diameter.h gtp.h smf.h smf_server.cpp smf_server.h network.h packet.h s1ap.h sctp_client.h sctp_server.h security.h sync.h telecom.h udp_client.h udp_server.h utils.h
	$(G++) -c -o smf_server.o smf_server.cpp        

mysql.o:	mysql.cpp mysql.h utils.h
	$(G++) -c -o mysql.o mysql.cpp

network.o:	diameter.h gtp.h network.cpp network.h packet.h s1ap.h utils.h 
	$(G++) -c -o network.o network.cpp

packet.o:	diameter.h gtp.h packet.cpp packet.h s1ap.h utils.h
	$(G++) -c -o packet.o packet.cpp

pgw.o:	diameter.h gtp.h network.h packet.h pgw.cpp pgw.h s1ap.h sync.h udp_client.h udp_server.h utils.h
	$(G++) -c -o pgw.o pgw.cpp

pgw_server.o:	diameter.h gtp.h network.h packet.h pgw.h pgw_server.cpp pgw_server.h s1ap.h sync.h udp_client.h udp_server.h utils.h
	$(G++) -c -o pgw_server.o pgw_server.cpp

ran.o: ports.h diameter.h gtp.h network.h packet.h ran.cpp ran.h s1ap.h sctp_client.h security.h sync.h telecom.h tun.h udp_client.h udp_server.h utils.h
	$(G++) -c -o ran.o ran.cpp

ran_simulator.o: ports.h diameter.h gtp.h network.h packet.h ran.h ran_simulator.cpp ran_simulator.h s1ap.h sctp_server.h sctp_client.h security.h sync.h telecom.h tun.h udp_client.h udp_server.h utils.h
	$(G++) -c -o ran_simulator.o ran_simulator.cpp

s1ap.o:	s1ap.cpp s1ap.h utils.h
	$(G++) -c -o s1ap.o s1ap.cpp

sctp_client.o:	diameter.h gtp.h network.h packet.h s1ap.h sctp_client.cpp sctp_client.h utils.h
	$(G++) -c -o sctp_client.o sctp_client.cpp

sctp_server.o:	diameter.h gtp.h network.h packet.h s1ap.h sctp_server.cpp sctp_server.h sync.h utils.h
	$(G++) -c -o sctp_server.o sctp_server.cpp	

security.o:	diameter.h gtp.h packet.h s1ap.h security.cpp security.h utils.h
	$(G++) -c -o security.o security.cpp

sgw.o:	diameter.h gtp.h network.h packet.h s1ap.h sgw.cpp sgw.h sync.h udp_client.h udp_server.h utils.h
	$(G++) -c -o sgw.o sgw.cpp

sgw_server.o:	diameter.h gtp.h network.h packet.h s1ap.h sgw.h sgw_server.cpp sgw_server.h sync.h udp_client.h udp_server.h utils.h
	$(G++) -c -o sgw_server.o sgw_server.cpp

sink.o: ports.h diameter.h gtp.h network.h packet.h s1ap.h sink.cpp sink.h tun.h udp_client.h udp_server.h utils.h
	$(G++) -c -o sink.o sink.cpp

sink_server.o: ports.h diameter.h gtp.h network.h packet.h s1ap.h sink.h sink_server.cpp sink_server.h tun.h udp_client.h udp_server.h utils.h
	$(G++) -c -o sink_server.o sink_server.cpp	

sync.o:	sync.cpp sync.h utils.h
	$(G++) -c -o sync.o sync.cpp

telecom.o:	telecom.cpp telecom.h utils.h
	$(G++) -c -o telecom.o telecom.cpp

tun.o:	diameter.h gtp.h network.h packet.h s1ap.h tun.cpp tun.h utils.h
	$(G++) -c -o tun.o tun.cpp

udp_client.o:	diameter.h gtp.h network.h packet.h s1ap.h udp_client.cpp udp_client.h utils.h
	$(G++) -c -o udp_client.o udp_client.cpp

udp_server.o:	diameter.h gtp.h network.h packet.h s1ap.h udp_server.cpp udp_server.h utils.h
	$(G++) -c -o udp_server.o udp_server.cpp

utils.o:	utils.cpp utils.h
	$(G++) -c -o utils.o utils.cpp
upf.o: ports.h diameter.h gtp.h network.h packet.h upf.cpp s1ap.h sync.h udp_client.h udp_server.h utils.h
	$(G++) -c -o upf.o upf.cpp

upf_server.o: ports.h diameter.h gtp.h network.h packet.h upf_server.cpp upf_server.h upf.h s1ap.h sync.h udp_client.h udp_server.h utils.h
	$(G++) -c -o upf_server.o upf_server.cpp

udm.o: ports.h diameter.h gtp.h udm.cpp udm.h mysql.h network.h packet.h s1ap.h sctp_server.h sctp_client.h sync.h utils.h
	$(G++) -c -o udm.o udm.cpp 

udm_server.o: ports.h diameter.h gtp.h udm.h udm_server.cpp udm_server.h mysql.h network.h packet.h s1ap.h sctp_server.h sctp_client.h sync.h utils.h
	$(G++) -c -o udm_server.o udm_server.cpp

#-------------------------------------------#



#---------Prerequisites and Recipes---------#

HSS_P = diameter.o gtp.o hss.o hss_server.o mysql.o network.o packet.o s1ap.o sctp_server.o sync.o utils.o
HSS_R = $(G++) -o hss.out $(HSS_P) -pthread `mysql_config --cflags --libs`

AUSF_P = diameter.o gtp.o ausf.o ausf_server.o mysql.o network.o packet.o s1ap.o sctp_server.o sctp_client.o sync.o utils.o
AUSF_R = $(G++) -o ausf.out $(AUSF_P) -pthread -lppconsul -ljsoncpp -lnghttp2_asio -lboost_system -lcrypto -lssl `mysql_config --cflags --libs`


AMF_P = diameter.o gtp.o amf.o amf_server.o network.o packet.o s1ap.o sctp_client.o sctp_server.o security.o sync.o telecom.o udp_client.o utils.o
AMF_R = $(G++) -o amf.out $(AMF_P) -pthread -lppconsul -lnghttp2_asio -ljsoncpp -lboost_system -lcrypto -lssl


MME_P = diameter.o gtp.o mme.o mme_server.o network.o packet.o s1ap.o sctp_client.o sctp_server.o security.o sync.o telecom.o udp_client.o utils.o
MME_R = $(G++) -o mme.out $(MME_P) -pthread -lcrypto


SMF_P = diameter.o gtp.o smf.o smf_server.o network.o packet.o s1ap.o sctp_client.o sctp_server.o security.o sync.o telecom.o udp_client.o udp_server.o utils.o
SMF_R = $(G++) -o smf.out $(SMF_P) -pthread -lppconsul -lnghttp2_asio -ljsoncpp -lboost_system -lcrypto -lssl

PGW_P = diameter.o gtp.o network.o packet.o pgw.o pgw_server.o s1ap.o sync.o udp_client.o udp_server.o utils.o
PGW_R = $(G++) -o pgw.out $(PGW_P) -pthread -lnghttp2_asio -lboost_system -lcrypto -lssl

RAN_P = diameter.o gtp.o network.o packet.o ran.o ran_simulator.o s1ap.o sctp_client.o sctp_server.o security.o sync.o telecom.o tun.o udp_client.o udp_server.o utils.o
RAN_R = $(G++) -o ransim.out $(RAN_P) -pthread -lppconsul -lcrypto -ljsoncpp -lnghttp2_asio -lboost_system -lssl

SGW_P = diameter.o gtp.o network.o packet.o s1ap.o sgw.o sgw_server.o sync.o udp_client.o udp_server.o utils.o
SGW_R = $(G++) -o sgw.out $(SGW_P) -pthread

SINK_P = diameter.o gtp.o network.o packet.o s1ap.o sink.o sink_server.o tun.o udp_client.o udp_server.o utils.o
SINK_R = $(G++) -o sink.out $(SINK_P) -pthread -lppconsul -lnghttp2_asio -ljsoncpp -lboost_system -lcrypto -lssl

UPF_P = diameter.o gtp.o network.o packet.o upf.o upf_server.o s1ap.o sync.o udp_client.o udp_server.o utils.o
UPF_R = $(G++) -o upf.out $(UPF_P) -pthread -lppconsul -lnghttp2_asio -lboost_system -ljsoncpp -lcrypto -lssl

UDM_P = diameter.o gtp.o udm.o udm_server.o mysql.o network.o packet.o s1ap.o sctp_server.o sctp_client.o sync.o utils.o
UDM_R = $(G++) -o udm.out $(UDM_P) -pthread -lppconsul `mysql_config --cflags --libs` -lcrypto -ljsoncpp -lnghttp2_asio -lboost_system -lcrypto -lssl

#-------------------------------------------#


#---------------Executable files------------#

hss.out:	$(HSS_P)
	$(HSS_R)

ausf.out:	$(AUSF_P)
	$(AUSF_R)


mme.out:	$(MME_P)
	$(MME_R)

smf.out:	$(SMF_P)
	$(SMF_R)

amf.out:	$(AMF_P)
	$(AMF_R)
	
pgw.out:	$(PGW_P)
	$(PGW_R)

ransim.out:	$(RAN_P)
	$(RAN_R)

sgw.out:	$(SGW_P)
	$(SGW_R)

sink.out:	$(SINK_P)	
	$(SINK_R)

upf.out:    $(UPF_P)
	$(UPF_R)

udm.out: $(UDM_P)
	$(UDM_R)

#-------------------------------------------#


#------------------Cleaner------------------#

clean:
	rm -f *~ *.o *.out

#-------------------------------------------#


#--------------Special Commands-------------#

# make -k (To keep going on even after encountering errors in making a former target)

#-------------------------------------------#
