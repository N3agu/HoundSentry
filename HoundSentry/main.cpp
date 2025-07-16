#include "ProcessUtils.h"
#include "Monitor.h"
#include <iostream>
#include <windows.h>
#include <csignal>

using std::cout;
using std::cerr;
using std::endl;

Monitor* g_Monitor = nullptr;

void SignalHandler(int signal) {
	if (signal == SIGINT && g_Monitor) {
		cout << "\n[INFO] Stopping trace..." << endl;
		g_Monitor->Stop();
	}
}

int main() {
	cout << R"( _   _                       _ ____             _              
| | | | ___  _   _ _ __   __| / ___|  ___ _ __ | |_ _ __ _   _ 
| |_| |/ _ \| | | | '_ \ / _` \___ \ / _ \ '_ \| __| '__| | | |
|  _  | (_) | |_| | | | | (_| |___) |  __/ | | | |_| |  | |_| |
|_| |_|\___/ \__,_|_| |_|\__,_|____/ \___|_| |_|\__|_|   \__, |
                                                         |___/ 
)";
	cout << "\t    The Backbone of ChangeHound - by N3agu\n\n";

	if (!IsProcessElevated()) {
		cerr << "[ERROR] This program requires administrator privileges." << endl;
		return 1;
	}

	signal(SIGINT, SignalHandler);

	Monitor monitor;
	g_Monitor = &monitor;

	cout << "[INFO] Starting ETW listener..." << endl;
	if (!monitor.Start()) {
		cerr << "[ERROR] Failed to start the ETW monitor." << endl;
		return 1;
	}

	monitor.WaitForExit();

	cout << "[INFO] Program has finished." << endl;
	return 0;
}