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
		wchar_t buf[256] = { 0 };
		_snwprintf_s(buf, 255, _TRUNCATE, L"Error connecting to SIM: %08x", hr);
		connect_result_ = buf;
		return hr;
	}

	hr = buildDefinition();
	if (FAILED(hr)) {
		return hr;
	}

	connected_ = true;
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

void SimulatorInterface::setSimData(const SimData* simData) {
	data_ = *simData;
	has_data_ = true;
	callbacks_->onSimDataUpdated();
}

void SimulatorInterface::onSimDisconnect() {
	connected_ = false;
	has_data_ = false;
	callbacks_->onSimDisconnect();
}

HRESULT SimulatorInterface::pollSimulator() {
	winfx::DebugOut(L"Requesting data from simulator...\n");
	HRESULT hr = SimConnect_RequestDataOnSimObjectType(sim_, REQUEST_1, DEFINITION_1, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
	if (FAILED(hr)) {
		winfx::DebugOut(L"RequestData failed with error %08x\n", hr);
	}
	SimConnect_CallDispatch(sim_, SimDispatchProc, this);
	if (message_ordinal_ % 4 == 0)
		broadcaster_->broadcastPositionReport(&data_);
	broadcaster_->broadcastAttitudeReport(&data_);
	message_ordinal_++;
	return S_OK;
}

static void CALLBACK SimDispatchProc(SIMCONNECT_RECV* recv_data, DWORD cbData, void* pContext) {
	SimulatorInterface* sim = (SimulatorInterface*)pContext;
	winfx::DebugOut(L"SimDispatchProc: %lx\n", recv_data->dwID);
	const SIMCONNECT_RECV_OPEN* open_data;
	const SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE* object_data;

	switch (recv_data->dwID) {
	case SIMCONNECT_RECV_ID_OPEN:
		winfx::DebugOut(L"SIMCONNECT_RECV_ID_OPEN\n");
		open_data = (SIMCONNECT_RECV_OPEN*)recv_data;
		winfx::DebugOut(L"RECV_OPEN: %S SIM VER: %d.%d\n", open_data->szApplicationName,
			open_data->dwApplicationBuildMajor, open_data->dwApplicationBuildMinor);
		break;
	case SIMCONNECT_RECV_ID_QUIT:
		winfx::DebugOut(L"SIMCONNECT_RECV_ID_QUIT\n");
		sim->onSimDisconnect();
		break;
	case SIMCONNECT_RECV_ID_EXCEPTION:
		//SIMCONNECT_RECV_EXCEPTION* except = (SIMCONNECT_RECV_EXCEPTION*)recv_data;
		break;
	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
		object_data = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)recv_data;
		winfx::DebugOut(L"SIMCONNECT_RECIV_ID_SIMOBJECT_DATA_BYTYPE: dwRequestID = %d\n", object_data->dwRequestID);
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

