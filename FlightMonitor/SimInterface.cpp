// Copyright(C) 2020 Alan Pearson
//
// This program is free software : you can redistribute it and /or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see < https://www.gnu.org/licenses/>.

#include "framework.h"
#include "winfx.h"
#include "MainWindow.h"
#include "ForeFlightBroadcaster.h"
#include "SimInterface.h"

constexpr DWORD REQUEST_1 = 0;
constexpr DWORD DEFINITION_1 = 0;

static void CALLBACK SimDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext);

#define CHECK_OR_FAIL(f) { \
  HRESULT hr = (f); \
  if (!SUCCEEDED(hr)) { \
    winfx::DebugOut(L"Error adding to data definition: %08x", hr); \
	return hr; \
  } \
}

HRESULT SimulatorInterface::connectSim(HWND hwnd) {
	winfx::DebugOut(L"Attempting to connect to sim\n");
	HRESULT hr = SimConnect_Open(&sim_, "FlightMonitor", hwnd, 0, 0, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL);
	if (FAILED(hr)) {
		return hr;
	}

	hr = buildDefinition();
	if (FAILED(hr)) {
		SimConnect_Close(sim_);
		sim_ = INVALID_HANDLE_VALUE;
		return hr;
	}

	state_ = SimInterfaceConnected;
	return S_OK;
}

HRESULT SimulatorInterface::buildDefinition() {
	CHECK_OR_FAIL(SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "GPS POSITION ALT", "meters"));
	CHECK_OR_FAIL(SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "GPS POSITION LAT", "degrees"));
	CHECK_OR_FAIL(SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "GPS POSITION LON", "degrees"));
	CHECK_OR_FAIL(SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "GPS GROUND TRUE TRACK", "degrees"));
	CHECK_OR_FAIL(SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "GPS GROUND SPEED", "meters per second"));
	CHECK_OR_FAIL(SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "PLANE PITCH DEGREES", "degrees"));
	CHECK_OR_FAIL(SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "PLANE BANK DEGREES", "degrees"));
	CHECK_OR_FAIL(SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "PLANE HEADING DEGREES TRUE", "degrees"));
	return S_OK;
}

static std::map<SimulatorInterfaceState, std::wstring> stateMessages = {
	{SimInterfaceDisconnected, L"Attempting to connect to simulator"},
	{SimInterfaceConnected, L"Connected to simulator"},
	{SimInterfaceReceivingData, L"Recieving data from simulator"},
	{SimInterfaceInFlight, L"In Flight"}
};

const std::wstring& SimulatorInterface::getStatusMessage() const {
	return stateMessages[state_];
}

void SimulatorInterface::setSimData(const SimData* simData) {
	data_ = *simData;
	if (!positionIsValid()) {
		state_ = SimInterfaceReceivingData;
	} else {
		state_ = SimInterfaceInFlight;
	}

	// Notify listeners of new data
	for (SimulatorCallbacks* callback : callbacks_) {
		callback->onSimDataUpdated();
	}
}

void SimulatorInterface::onSimDisconnect() {
	close();
}

void SimulatorInterface::close() {
	SimConnect_Close(sim_);
	sim_ = INVALID_HANDLE_VALUE;
	state_ = SimInterfaceDisconnected;
	for (SimulatorCallbacks* callback : callbacks_) {
		callback->onSimDisconnect();
	}
}

HRESULT SimulatorInterface::pollSimulator() {
	if (!isConnected()) {
		winfx::DebugOut(L"Invalid call to pollSimulator when not connected.\n");
		return E_FAIL;
	}
	winfx::DebugOut(L"Requesting data from simulator...\n");
	HRESULT hr = SimConnect_RequestDataOnSimObjectType(sim_, REQUEST_1, DEFINITION_1, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
	if (FAILED(hr)) {
		winfx::DebugOut(L"RequestData failed with error %08x\n", hr);
		close();
		return hr;
	}
	SimConnect_CallDispatch(sim_, SimDispatchProc, this);
	return S_OK;
}

bool SimulatorInterface::positionIsValid() {
	// A hack to determine if the GPS position is a valid position. While on the
	// loading screen MSFS returns a position approximately at lat/lon 0,0. This 
	// check rejects that location. Otherwise, ForeFlight would draw a track from 
	// the last flight position to the middle of the Atlantic Ocean when the flight
	// ends.
	return !((data_.gps_lat < 0.1 && data_.gps_lat > -0.1) &&
		(data_.gps_lon < 0.1 && data_.gps_lon > -0.1) &&
		data_.gps_alt < 10);
}

static void CALLBACK SimDispatchProc(SIMCONNECT_RECV* recv_data, DWORD cbData, 
									 void* pContext) {
	SimulatorInterface* sim = (SimulatorInterface*)pContext;
	const SIMCONNECT_RECV_OPEN* open_data = NULL;
	const SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE* object_data = NULL;
	const SIMCONNECT_RECV_EXCEPTION* except = NULL;

	winfx::DebugOut(L"SimDispatchProc: %lx\n", recv_data->dwID);

	switch (recv_data->dwID) {
	case SIMCONNECT_RECV_ID_OPEN:
		winfx::DebugOut(L"SIMCONNECT_RECV_ID_OPEN\n");
		open_data = (SIMCONNECT_RECV_OPEN*)recv_data;
		winfx::DebugOut(L"RECV_OPEN: %S SIM VER: %d.%d\n", 
			open_data->szApplicationName,
			open_data->dwApplicationBuildMajor, 
			open_data->dwApplicationBuildMinor);
		break;
	case SIMCONNECT_RECV_ID_QUIT:
		winfx::DebugOut(L"SIMCONNECT_RECV_ID_QUIT\n");
		sim->onSimDisconnect();
		break;
	case SIMCONNECT_RECV_ID_EXCEPTION:
		except = (SIMCONNECT_RECV_EXCEPTION*)recv_data;
		winfx::DebugOut(L"SIMCONNECT_RECV_ID_EXCEPTION: dwException = %08x\n", 
			except->dwException);
		break;
	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
		object_data = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)recv_data;
		winfx::DebugOut(L"SIMCONNECT_RECIV_ID_SIMOBJECT_DATA_BYTYPE: dwRequestID = %d\n", 
			object_data->dwRequestID);
		if (object_data->dwRequestID == REQUEST_1) {
			DWORD object_id = object_data->dwObjectID;
			const SimData* const sim_data = (SimData*)&object_data->dwData;
			sim->setSimData(sim_data);
		}
		break;
	default:
		break;
	}
}

