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
#include "Resource.h"

// we need commctrl v6 for LoadIconMetric()
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;

#define HANDLE_WMAPP_NOTIFYCALLBACK(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (DWORD)LOWORD(lParam), winfx::Point(LOWORD(wParam), HIWORD(wParam))), 0L)

constexpr int kPollTimerIntervalMs = 1000 / kAttitueReportsPerSecond;
constexpr int kReconnectTimerIntervalMs = 5000;

// Ugly hack. The path to the executable is stored by the Shell when you call
// Shell_NotifyIcon (https://docs.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-notifyicondataa#troubleshooting)
// Since the Debug and Release versions compile to different locations, they have
// to have different GUIDs. :-[
//
#ifdef _DEBUG
class __declspec(uuid("5a53c7ca-f57d-4ea2-ab43-4ab65994cb61")) AppIcon;
#else
class __declspec(uuid("f0d5c17f-e9d7-42ce-ad26-b50a93f93f06")) AppIcon;
#endif

void MainWindow::modifyWndClass(WNDCLASSEXW& wc) {
	wc.lpszMenuName = MAKEINTRESOURCE(IDC_FLIGHTMONITOR);
	wc.hIcon = ::LoadIcon(winfx::App::getSingleton().getInstance(), MAKEINTRESOURCE(IDI_AIRPLANE_GREEN));
	wc.hIconSm = ::LoadIcon(winfx::App::getSingleton().getInstance(), MAKEINTRESOURCE(IDI_AIRPLANE_GREEN));

}

bool MainWindow::create(LPWSTR pstrCmdLine, int nCmdShow) {
	// override create to always start hidden
	return Window::create(pstrCmdLine, SW_HIDE);
}

LRESULT MainWindow::handleWindowMessage(HWND hwndParam, UINT uMsg, 
										WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		HANDLE_MSG(hwndParam, WM_ACTIVATE, onActivate);
		HANDLE_MSG(hwndParam, WM_COMMAND, onCommand);
		HANDLE_MSG(hwndParam, WM_DESTROY, onDestroy);
		HANDLE_MSG(hwndParam, WM_PAINT, onPaint);
		HANDLE_MSG(hwndParam, WM_TIMER, onTimer);
		HANDLE_MSG(hwndParam, WMAPP_NOTIFYCALLBACK, onNotifyCallback);
	}
	return Window::handleWindowMessage(hwndParam, uMsg, wParam, lParam);
}

LRESULT MainWindow::onCreate(HWND hwndParam, LPCREATESTRUCT lpCreateStruct) {
	// Create a broadcast UDP socket
	broadcaster_.init();

	// Attempt to connect to the simulator.
	if (FAILED(connectSim())) {
		// Set a timer to attempt to periodically retry connecting
		SetTimer(hwndParam, ID_TIMER_SIM_CONNECT, kReconnectTimerIntervalMs, NULL);
	}

	if (!AddNotificationIcon()) {
		winfx::DebugOut(L"Failed to add notification icon\n");
		return FALSE;
	}

	return winfx::Window::onCreate(hwndParam, lpCreateStruct);
}

LRESULT MainWindow::onActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized) {
	winfx::DebugOut(L"onActivate state=%08x, fMinimized=%d\n", state, fMinimized);
	if (state == WA_INACTIVE && fMinimized) {
		ShowWindow(hwnd, SW_HIDE);
		return 0;
	}
	return 1;
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

	case IDM_SHOWWINDOW:
		ShowWindow(hwnd, SW_SHOWNORMAL);
		break;

	case IDM_EXIT:
		destroy();
		break;
	}
}

void MainWindow::onNotifyCallback(HWND hwndParam, UINT idNotify, winfx::Point point) {
	winfx::DebugOut(L"onNotifyCallback: %d\n", idNotify);
	switch (idNotify) {
	case NIN_SELECT:
		break;

	case WM_LBUTTONDBLCLK:
		ShowWindow(hwnd, SW_SHOWNORMAL);
		break;

	case WM_CONTEXTMENU:
		ShowContextMenu(hwnd, point);
		break;
	}
}

void MainWindow::ShowContextMenu(HWND hwnd, winfx::Point point) {
	HMENU hMenu = LoadMenu(winfx::App::getSingleton().getInstance(), MAKEINTRESOURCE(IDC_CONTEXTMENU));
	if (hMenu) {
		HMENU hSubMenu = GetSubMenu(hMenu, 0);
		if (hSubMenu) {
			// our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
			SetForegroundWindow(hwnd);

			// respect menu drop alignment
			UINT uFlags = TPM_RIGHTBUTTON;
			if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
				uFlags |= TPM_RIGHTALIGN;
			} else {
				uFlags |= TPM_LEFTALIGN;
			}

			TrackPopupMenuEx(hSubMenu, uFlags, point.x, point.y, hwnd, NULL);
		}
		DestroyMenu(hMenu);
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
	DeleteNotificationIcon();
	sim_.close();
	PostQuitMessage(0);
}

BOOL MainWindow::AddNotificationIcon() {
	HRESULT hr;
	NOTIFYICONDATAW nid = { sizeof(nid) };
	nid.hWnd = hwnd;
	// add the icon, setting the icon, tooltip, and callback message.
	// the icon will be identified with the GUID
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
	nid.guidItem = __uuidof(AppIcon);
	nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
	hr = LoadIconMetric(winfx::App::getSingleton().getInstance(),
		MAKEINTRESOURCE(IDI_AIRPLANE_RED), LIM_SMALL, &nid.hIcon);
	if (FAILED(hr)) {
		winfx::DebugOut(L"Error loading icon: %08x\n", hr);
		return FALSE;
	}
	LoadString(winfx::App::getSingleton().getInstance(), IDS_NOTCONNECTED,
		nid.szTip, ARRAYSIZE(nid.szTip));

	int tries = 0;
	for (;;) {
		if (Shell_NotifyIconW(NIM_ADD, &nid)) {
			break;
		}
		winfx::DebugOut(L"AddNotifyIcon failed: %08X\n", GetLastError());
		if (++tries == 1) {
			// Try to delete the icon and re-create it
			winfx::DebugOut(L"Deleting icon and retrying\n");
			DeleteNotificationIcon();
		} else {
			// Give up...
			winfx::DebugOut(L"Failed to create icon after retry.\n");
			return FALSE;
		}
	}

	// NOTIFYICON_VERSION_4 is prefered
	nid.uVersion = NOTIFYICON_VERSION_4;
	return Shell_NotifyIconW(NIM_SETVERSION | NIM_ADD, &nid);
}

BOOL MainWindow::DeleteNotificationIcon() {
	NOTIFYICONDATAW nid = { sizeof(nid) };
	nid.uFlags = NIF_GUID;
	nid.guidItem = __uuidof(AppIcon);
	return Shell_NotifyIconW(NIM_DELETE, &nid);
}

HRESULT MainWindow::connectSim() {
	HRESULT hr = sim_.connectSim(hwnd);
	if (SUCCEEDED(hr)) {
		SetTimer(hwnd, ID_TIMER_POLL_SIM, kPollTimerIntervalMs, NULL);
	}
	return hr;
}

void MainWindow::onSimDataUpdated(const SimData* data) {
	// When there is new data, invalidate the window to force a repaint.
	InvalidateRect(hwnd, NULL, TRUE);
}

void MainWindow::onStateChange(SimulatorInterfaceState state) {
	int icon;
	int tooltip;

	switch (state) {
	case SimInterfaceDisconnected:
		icon = IDI_AIRPLANE_RED;
		tooltip = IDS_NOTCONNECTED;
		break;

	case SimInterfaceConnected:
	case SimInterfaceReceivingData:
		icon = IDI_AIRPLANE_YELLOW;
		tooltip = IDS_CONNECTED;
		break;

	case SimInterfaceInFlight:
		icon = IDI_AIRPLANE_GREEN;
		tooltip = IDS_INFLIGHT;
		break;

	default:
		return;
	}

	NOTIFYICONDATAW nid = { sizeof(nid) };
	nid.hWnd = hwnd;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_GUID;
	nid.guidItem = __uuidof(AppIcon);
	LoadIconMetric(winfx::App::getSingleton().getInstance(),
		MAKEINTRESOURCE(icon), LIM_SMALL, &nid.hIcon);
	LoadString(winfx::App::getSingleton().getInstance(), tooltip,
		nid.szTip, ARRAYSIZE(nid.szTip));

	if (!Shell_NotifyIconW(NIM_MODIFY, &nid)) {
		winfx::DebugOut(L"Failed to modify notify icon.\n");
	}
}

void MainWindow::onSimDisconnect() {
	// Stop the polling timer
	KillTimer(hwnd, ID_TIMER_POLL_SIM);

	// Set a timer to attempt to periodically retry connecting
	SetTimer(hwnd, ID_TIMER_SIM_CONNECT, kReconnectTimerIntervalMs, NULL);

	InvalidateRect(hwnd, NULL, TRUE);
}
