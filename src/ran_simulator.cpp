#include <atomic>
#include "ran_simulator.h"

time_t g_start_time;
int g_threads_count;
uint64_t g_req_dur;
uint64_t g_run_dur;

atomic<int> g_tot_regs;
atomic<uint64_t> g_tot_regstime;
atomic<uint64_t> ue_registration_tot_regstime;
atomic<uint64_t> ue_session_tot_regstime;
atomic<uint64_t> ue_detach_tot_regstime;

vector<thread> g_umon_thread;
vector<thread> g_dmon_thread;
vector<thread> g_threads;
thread g_rtt_thread;
TrafficMonitor g_traf_mon;

void utraffic_monitor() {
	UdpClient sgw_s1_client;
	
	sgw_s1_client.set_client(g_trafmon_ip_addr);
	while (1) {
		g_traf_mon.handle_uplink_udata(sgw_s1_client);
	}
	
}

void dtraffic_monitor() {
	while (1) {
		g_traf_mon.handle_downlink_udata();		
	}
}

void ping(){
	string cmd;
	
	cmd = "ping -I 172.16.1.3 172.16.0.2 -c 60 | grep \"^rtt\" >> ping.txt";
	cout << cmd << endl;
	system(cmd.c_str());
}

void simulate(int arg) {
	CLOCK::time_point mstart_time;
	CLOCK::time_point mstop_time;
	MICROSECONDS mtime_diff_us;		
	Ran ran;
	int status;
	int ran_num;
	bool ok;
	bool time_exceeded;

	ran_num = arg;
	time_exceeded = false;
	ran.init(ran_num);
	ran.conn_mme();

	CLOCK::time_point ue_registration_time_end;
	CLOCK::time_point ue_session_time_end;
	CLOCK::time_point ue_detach_time_start;
	MICROSECONDS ue_registration_diff_us;
	MICROSECONDS ue_session_diff_us;
	MICROSECONDS ue_detach_diff_us;

	while (1) {
		// Run duration check
		g_utils.time_check(g_start_time, g_req_dur, time_exceeded);
		if (time_exceeded) {
			break;
		}

		// Start time
		mstart_time = CLOCK::now();	
		
		// Initial attach
		ran.initial_attach();

		// Authentication
		ok = ran.authenticate();
		if (!ok) {
			TRACE(cout << "ransimulator_simulate:" << " autn failure" << endl;)
			return;
		}

		// Set security
		ok = ran.set_security();
		if (!ok) {
			TRACE(cout << "ransimulator_simulate:" << " security setup failure" << endl;)
			return;
		}

		ue_registration_time_end = CLOCK::now();

		// Set eps session
		ok = ran.set_eps_session(g_traf_mon);
		if (!ok) {
			TRACE(cout << "ransimulator_simulate:" << " eps session setup failure" << endl;)
			return;
		}

		ue_session_time_end = CLOCK::now();

		/*
		// To find RTT
		if (ran_num == 0) {
			g_rtt_thread = thread(ping);
			g_rtt_thread.detach();		
		}
		*/ 

		/* Data transfer */
		ran.transfer_data(g_req_dur);

		ue_detach_time_start = CLOCK::now();
		// Detach
		ok = ran.detach();
		if (!ok) {
			TRACE(cout << "ransimulator_simulate:" << " detach failure" << endl;)
			return;
		}

		// Stop time
		mstop_time = CLOCK::now();
		
		// Response time
		ue_registration_diff_us = std::chrono::duration_cast<MICROSECONDS>(ue_registration_time_end - mstart_time);
		ue_session_diff_us = std::chrono::duration_cast<MICROSECONDS>(ue_session_time_end - ue_registration_time_end);
		ue_detach_diff_us = std::chrono::duration_cast<MICROSECONDS>(mstop_time - ue_detach_time_start);
		mtime_diff_us = std::chrono::duration_cast<MICROSECONDS>(mstop_time - mstart_time);

		/* Updating performance metrics */
		g_tot_regs++;
		ue_registration_tot_regstime += ue_registration_diff_us.count();
		ue_session_tot_regstime += ue_session_diff_us.count();
		ue_detach_tot_regstime += ue_detach_diff_us.count();
		g_tot_regstime += mtime_diff_us.count();
	}
}

void check_usage(int argc) {
	if (argc < 3) {
		TRACE(cout << "Usage: ./<ran_simulator_exec> THREADS_COUNT DURATION" << endl;)
		g_utils.handle_type1_error(-1, "Invalid usage error: ransimulator_checkusage");
	}
}

void init(char *argv[]) {
	g_start_time = time(0);
	g_threads_count = atoi(argv[1]);
	g_req_dur = atoi(argv[2]);
	g_tot_regs = 0;
	g_tot_regstime = 0;
	ue_registration_tot_regstime = 0;
	ue_session_tot_regstime = 0;
	ue_detach_tot_regstime = 0;
	g_umon_thread.resize(NUM_MONITORS);
	g_dmon_thread.resize(NUM_MONITORS);
	g_threads.resize(g_threads_count);
	signal(SIGPIPE, SIG_IGN);
}

void run() {
	int i;

	/* Tun */
	g_traf_mon.tun.set_itf("tun1", "172.16.0.1/16");
	g_traf_mon.tun.conn("tun1");

	/* Traffic monitor server */
	TRACE(cout << "Traffic monitor server started" << endl;)
	g_traf_mon.server.run(g_trafmon_ip_addr, g_trafmon_port);	

	// Uplink traffic monitor
	for (i = 0; i < NUM_MONITORS; i++) {
		g_umon_thread[i] = thread(utraffic_monitor);
		g_umon_thread[i].detach();		
	}

	// Downlink traffic monitor
	for (i = 0; i < NUM_MONITORS; i++) {
		g_dmon_thread[i] = thread(dtraffic_monitor);
		g_dmon_thread[i].detach();			
	}
	
	// Simulator threads
	for (i = 0; i < g_threads_count; i++) {
		g_threads[i] = thread(simulate, i);
	}	
	for (i = 0; i < g_threads_count; i++) {
		if (g_threads[i].joinable()) {
			g_threads[i].join();
		}
	}	
}

void print_results() {
	g_run_dur = difftime(time(0), g_start_time);
	
	cout << "\n\n";
	cout << "Requested duration has ended. Finishing the program." << endl;
	cout << "Total number of registrations is " << g_tot_regs << endl;
	cout << "Total time for registrations is " << g_tot_regstime * 1e-6 << " seconds" << endl;

	cout << "Total time for UE registrations is " << ue_registration_tot_regstime * 1e-6 << " seconds" << endl;
	cout << "Average time for UE registrations is " << ((double)ue_registration_tot_regstime / g_tot_regs) * 1e-6 << " seconds" << endl;

	cout << "Total time for UE session is " << ue_session_tot_regstime * 1e-6 << " seconds" << endl;
	cout << "Average time for UE session is " << ((double)ue_session_tot_regstime / g_tot_regs) * 1e-6 << " seconds" << endl;

	cout << "Total time for UE detach is " << ue_detach_tot_regstime * 1e-6 << " seconds" << endl;
	cout << "Average time for UE detach is " << ((double)ue_detach_tot_regstime / g_tot_regs) * 1e-6 << " seconds" << endl;

	cout << "Total run duration is " << g_run_dur << " seconds" << endl;
	cout << "Latency is " << ((double)g_tot_regstime / g_tot_regs) * 1e-6 << " seconds" << endl;
	cout << "Throughput is " << ((double)g_tot_regs / g_run_dur) << endl;
}

int main(int argc, char *argv[]) {
	check_usage(argc);
	init(argv);
	run();
	print_results();
	return 0;
}
