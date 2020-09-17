#include "framework.h"
#include "winfx.h"
#include "FlightMonitorApp.h"
#include "ForeFlightBroadcaster.h"
#include "Resource.h"

bool FlightMonitorApp::initWindow(LPWSTR pwstrCmdLine, int nCmdShow) {
	if (ForeFlightBroadcaster::InitWinsock() != 0) {
		return false;
	}
	return mainWindow.create(pwstrCmdLine, nCmdShow);
}

FlightMonitorApp flightApp;
