#include "framework.h"
#include "winfx.h"
#include "Resource.h"

#define ID_TIMER_SIM_COMMECT 100
#define ID_TIMER_POLL_SIM    101

struct SimData {
	char    title[256] = { 0 };
	double  altimeter_setting = 0;
	double  altitude = 0;
	double  latitude = 0;
	double  longitude = 0;
};

class MainWindow : public winfx::Window {
public:
	MainWindow() : winfx::Window(winfx::loadString(IDC_FLIGHTMONITOR), winfx::loadString(IDS_APP_TITLE)) {}
	virtual void modifyWndClass(WNDCLASS& wc) override;
	virtual LRESULT handleWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual winfx::Size getDefaultWindowSize() override {
		return winfx::Size(400, 300);
	}

	void setSimData(const SimData* simData);
	LRESULT onCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) override;

protected:
	BOOL connectSim();
	void onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void onDestroy(HWND hwnd);
	void onPaint(HWND hwnd);
	void onTimer(HWND hwnd, UINT idTimer);

private:
	bool connected_ = false;
	bool has_data_ = false;
	HANDLE sim_ = INVALID_HANDLE_VALUE;
	std::wstring connect_result_;
	SimData data_;
};

class AboutDialog : public winfx::Dialog {
public:
	AboutDialog(Window* pwnd) : winfx::Dialog(pwnd, IDD_ABOUTBOX) {}
};

void MainWindow::modifyWndClass(WNDCLASS& wc)  {
	wc.lpszMenuName = MAKEINTRESOURCE(IDC_FLIGHTMONITOR);
}

LRESULT MainWindow::handleWindowMessage(HWND hwndParam, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		HANDLE_MSG(hwndParam, WM_COMMAND, onCommand);
		HANDLE_MSG(hwndParam, WM_DESTROY, onDestroy);
		HANDLE_MSG(hwndParam, WM_PAINT, onPaint);
		HANDLE_MSG(hwndParam, WM_TIMER, onTimer);
	}
	return Window::handleWindowMessage(hwndParam, uMsg, wParam, lParam);
}

#define REQUEST_1    0
#define DEFINITION_1 0

LRESULT MainWindow::onCreate(HWND hwndParam, LPCREATESTRUCT lpCreateStruct) {
	// Attempt to connect to the simulator.
	if (!connectSim()) {
		// Set a timer to attempt to periodically retry connecting
		SetTimer(hwndParam, ID_TIMER_SIM_COMMECT, 1000, NULL);
	}
	return winfx::Window::onCreate(hwndParam, lpCreateStruct);
}

void MainWindow::setSimData(const SimData* simData) {
	data_ = *simData;
	has_data_ = true;
	InvalidateRect(hwnd, NULL, TRUE);
}

void CALLBACK SimDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {
	MainWindow* pWnd = (MainWindow*)pContext;
	winfx::DebugOut(L"SimDispatchProc: %lx\n", pData->dwID);

	switch (pData->dwID) {
	case SIMCONNECT_RECV_ID_OPEN:
	{
		winfx::DebugOut(L"SIMCONNECT_RECV_ID_OPEN\n");
		// enter code to handle SimConnect version information received in a SIMCONNECT_RECV_OPEN structure.
		SIMCONNECT_RECV_OPEN* openData = (SIMCONNECT_RECV_OPEN*)pData;
		winfx::DebugOut(L"RECV_OPEN: %S SIM VER: %d.%d\n", openData->szApplicationName, openData->dwApplicationBuildMajor, openData->dwApplicationBuildMinor);
	} break;
	case SIMCONNECT_RECV_ID_EVENT:
		// enter code to handle events received in a SIMCONNECT_RECV_EVENT structure.
		//SIMCONNECT_RECV_EVENT* evt = (SIMCONNECT_RECV_EVENT*)pData;
		break;
	case SIMCONNECT_RECV_ID_EVENT_FILENAME:
		// enter code to handle event filenames received in a SIMCONNECT_RECV_EVENT_FILENAME
		// structure.
		//SIMCONNECT_RECV_EVENT_FILENAME* evt = (SIMCONNECT_RECV_EVENT_FILENAME*)pData;
		break;
	case SIMCONNECT_RECV_ID_EVENT_OBJECT_ADDREMOVE:
		// enter code to handle AI objects that have been added or removed, and received in a SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE
		// structure.
		//SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE* evt = (SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE*)pData;
		break;
	case SIMCONNECT_RECV_ID_EVENT_FRAME:
		// enter code to handle frame data received in a SIMCONNECT_RECV_EVENT_FRAME
		// structure.
		//SIMCONNECT_RECV_EVENT_FRAME* evt = (SIMCONNECT_RECV_EVENT_FRAME*)pData;
		break;
	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
		// enter code to handle object data received in a SIMCONNECT_RECV_SIMOBJECT_DATA structure.
		//SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;
		break;
	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE: {
		SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;
		winfx::DebugOut(L"dwRequestID = %d\n", pObjData->dwRequestID);
		if (pObjData->dwRequestID == REQUEST_1) {
			DWORD ObjectID = pObjData->dwObjectID;
			SimData* pS = (SimData*)&pObjData->dwData;
			winfx::DebugOut(L"\nObjectID=%d  Title=\"%S\"\nLat=%f  Lon=%f  Alt=%f  Altimeter=%.2f\n",
				ObjectID, pS->title, pS->latitude, pS->longitude, pS->altitude, pS->altimeter_setting);
			pWnd->setSimData(pS);
		}
		break;
	}

	case SIMCONNECT_RECV_ID_QUIT:
		// enter code to handle exiting the application
		break;
	case SIMCONNECT_RECV_ID_EXCEPTION:
		// enter code to handle errors received in a SIMCONNECT_RECV_EXCEPTION structure.
		//SIMCONNECT_RECV_EXCEPTION* except = (SIMCONNECT_RECV_EXCEPTION*)pData;
		break;
	case SIMCONNECT_RECV_ID_WEATHER_OBSERVATION:
		// enter tode to handle object data received in a SIMCONNECT_RECV_WEATHER_OBSERVATION structure.`
		//SIMCONNECT_RECV_WEATHER_OBSERVATION* pWxData = (SIMCONNECT_RECV_WEATHER_OBSERVATION*)pData;
		//const char* pszMETAR = (const char*)(pWxData + 1);
		break;
	default:
		// Enter similar case statements to handle the other types of data that can be received, including:
		// SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID,
		// SIMCONNECT_RECV_ID_RESERVED_KEY,
		// SIMCONNECT_RECV_ID_CUSTOM_ACTION
		// SIMCONNECT_RECV_ID_SYSTEM_STATE
		// SIMCONNECT_RECV_ID_CLOUD_STATE
		// enter code to handle the case where an unexpected message is received
		break;
	}
}

void MainWindow::onTimer(HWND hwndParam, UINT idTimer) {
	HRESULT hr;
	winfx::DebugOut(L"OnTimer\n");
	switch (idTimer) {
	case ID_TIMER_POLL_SIM:
		winfx::DebugOut(L"Requesting data...\n");
		hr = SimConnect_RequestDataOnSimObjectType(sim_, REQUEST_1, DEFINITION_1, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
		if (FAILED(hr)) {
			winfx::DebugOut(L"RequestData failed with error %08x\n", hr);
		}
		SimConnect_CallDispatch(sim_, SimDispatchProc, this);
		break;

	case ID_TIMER_SIM_COMMECT:
		if (connectSim()) {
			KillTimer(hwndParam, ID_TIMER_SIM_COMMECT);
		}
		break;
	}
}


BOOL MainWindow::connectSim() {
	winfx::DebugOut(L"Attempting to connect to sim\n");
	HRESULT hr = SimConnect_Open(&sim_, "FlightMonitor", hwnd, 0, 0, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL);
	if (SUCCEEDED(hr)) {
		hr = SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "Title", NULL, SIMCONNECT_DATATYPE_STRING256);
		hr = SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "Kohlsman setting hg", "inHg");
		hr = SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "Plane Altitude", "feet");
		hr = SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "Plane Latitude", "degrees");
		hr = SimConnect_AddToDataDefinition(sim_, DEFINITION_1, "Plane Longitude", "degrees");
		SetTimer(hwnd, ID_TIMER_POLL_SIM, 1000, NULL);
		connected_ = true;
	} else {
		wchar_t buf[256] = { 0 };
		_snwprintf_s(buf, 255, _TRUNCATE, L"Error connecting to SIM: %08x", hr);
		connect_result_ = buf;
	}

	return connected_;
}

void MainWindow::onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
	switch (id) {
	case ID_FLIGHT_CONNECT:
		connectSim();
		InvalidateRect(hwnd, NULL, TRUE);
		break;

	case IDM_ABOUT:
		AboutDialog(this).doDialogBox();
		break;

	case IDM_EXIT:
		destroy();
		break;
	}
}

void MainWindow::onPaint(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	HFONT hFont = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, 
		DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
		ANTIALIASED_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
	SelectObject(hdc, hFont);
	if (connected_) {
		SetTextColor(hdc, RGB(64, 255, 64));
		DrawText(hdc, TEXT("Connected"), -1, (LPRECT)winfx::Rect(10, 10, 300, 30), DT_NOCLIP);

		if (has_data_) {
			SetTextColor(hdc, RGB(0, 0, 0));
			wchar_t buf[1024];
			_snwprintf_s(buf, 1023, _TRUNCATE, L"ALT: %f", data_.altitude);
			DrawText(hdc, buf, -1, (LPRECT)winfx::Rect(10, 30, 300, 50), DT_NOCLIP);
			_snwprintf_s(buf, 1023, _TRUNCATE, L"LAT: %f", data_.latitude);
			DrawText(hdc, buf, -1, (LPRECT)winfx::Rect(10, 50, 300, 70), DT_NOCLIP);
			_snwprintf_s(buf, 1023, _TRUNCATE, L"LON: %f", data_.longitude);
			DrawText(hdc, buf, -1, (LPRECT)winfx::Rect(10, 70, 300, 90), DT_NOCLIP);
		}
	} else if (connect_result_.empty()) {
		SetTextColor(hdc, RGB(0, 0, 0));
		DrawText(hdc, TEXT("Not Connected"), -1, (LPRECT)winfx::Rect(10, 10, 300, 30), DT_NOCLIP);
	} else {
		SetTextColor(hdc, RGB(255, 0, 0));
		DrawText(hdc, connect_result_.c_str(), -1, (LPRECT)winfx::Rect(10, 10, 300, 30), DT_NOCLIP);
	}
	DeleteObject(hFont);
	EndPaint(hwnd, &ps);
}

void MainWindow::onDestroy(HWND hwnd) {
	PostQuitMessage(0);
}

class FlightMonitorApp : public winfx::App {
protected:
	MainWindow mainWindow;
public:
	virtual bool initWindow(LPWSTR pwstrCmdLine, int nCmdShow) ;
};

bool FlightMonitorApp::initWindow(LPWSTR pwstrCmdLine, int nCmdShow) {
	return mainWindow.create(pwstrCmdLine, nCmdShow);
}

FlightMonitorApp flightApp;
