#include "framework.h"
#include "winfx.h"
#include "MainWindow.h"
#include "ForeFlightBroadcaster.h"
#include "Resource.h"

void MainWindow::modifyWndClass(WNDCLASS& wc) {
	wc.lpszMenuName = MAKEINTRESOURCE(IDC_FLIGHTMONITOR);
}

LRESULT MainWindow::handleWindowMessage(HWND hwndParam, UINT uMsg, 
										WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		HANDLE_MSG(hwndParam, WM_COMMAND, onCommand);
		HANDLE_MSG(hwndParam, WM_DESTROY, onDestroy);
		HANDLE_MSG(hwndParam, WM_PAINT, onPaint);
		HANDLE_MSG(hwndParam, WM_TIMER, onTimer);
	}
	return Window::handleWindowMessage(hwndParam, uMsg, wParam, lParam);
}

LRESULT MainWindow::onCreate(HWND hwndParam, LPCREATESTRUCT lpCreateStruct) {
	// Create a broadcast UDP socket
	broadcaster_.init();

	// Attempt to connect to the simulator.
	if (FAILED(connectSim())) {
		// Set a timer to attempt to periodically retry connecting
		SetTimer(hwndParam, ID_TIMER_SIM_CONNECT, 1000, NULL);
	}

	return winfx::Window::onCreate(hwndParam, lpCreateStruct);
}

void MainWindow::onTimer(HWND hwndParam, UINT idTimer) {
	switch (idTimer) {
	case ID_TIMER_POLL_SIM:
		sim_.pollSimulator();
		break;

	case ID_TIMER_SIM_CONNECT:
		if (SUCCEEDED(connectSim())) {
			KillTimer(hwndParam, ID_TIMER_SIM_CONNECT);
		}
		break;
	}
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

	HFONT hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, FIXED_PITCH, TEXT("Consolas"));

	int posx = 40;
	auto DrawAttribute = [&](LPCWSTR format, double value) {
		wchar_t buf[1024];
		_snwprintf_s(buf, 1023, _TRUNCATE, format, value);
		DrawText(hdc, buf, -1, (LPRECT)winfx::Rect(10, posx, 300, posx + 20), 
			DT_NOCLIP);
		posx += 20;
	};

	// Draw the status message
	SelectObject(hdc, hFont);
	if (sim_.isConnected()) {
		SetTextColor(hdc, RGB(64, 255, 64));
	} else {
		SetTextColor(hdc, RGB(0, 0, 0));
	}
	DrawText(hdc, sim_.getStatusMessage().c_str(), -1, (LPRECT)winfx::Rect(10, 10, 300, 30),
		DT_NOCLIP);
	
	// Draw the position if available.
	const SimData* const data = sim_.getData();
	if (data) {
		SetTextColor(hdc, RGB(0, 0, 0));
		DrawAttribute(L"GPS ALT: %0.2f m", data->gps_alt);
		DrawAttribute(L"GPS LAT: %0.4f", data->gps_lat);
		DrawAttribute(L"GPS LON: %0.4f", data->gps_lon);
		DrawAttribute(L"GPS TRK: %0.1f", data->gps_track);
		DrawAttribute(L"GPS GS:  %0.1f m/s", data->gps_groundspeed);

		posx += 10;
		DrawAttribute(L"PITCH: %0.3f", data->pitch);
		DrawAttribute(L"BANK:  %0.3f", data->bank);
		DrawAttribute(L"HDG:   %0.1f", data->heading);
	}

	DeleteObject(hFont);
	EndPaint(hwnd, &ps);
}

void MainWindow::onDestroy(HWND hwnd) {
	sim_.close();
	PostQuitMessage(0);
}

HRESULT MainWindow::connectSim() {
	HRESULT hr = sim_.connectSim(hwnd);
	if (SUCCEEDED(hr)) {
		SetTimer(hwnd, ID_TIMER_POLL_SIM, 250, NULL);
	}
	return hr;
}

void MainWindow::onSimDataUpdated() {
	// When there is new data, invalidate the window to force a repaint.
	InvalidateRect(hwnd, NULL, TRUE);
}

void MainWindow::onSimDisconnect() {
	// Stop the polling timer
	KillTimer(hwnd, ID_TIMER_POLL_SIM);

	// Set a timer to attempt to periodically retry connecting
	SetTimer(hwnd, ID_TIMER_SIM_CONNECT, 1000, NULL);

	InvalidateRect(hwnd, NULL, TRUE);
}
