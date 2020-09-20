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

#pragma once

#include "framework.h"
#include "winfx.h"
#include "ForeFlightBroadcaster.h"
#include "SimInterface.h"
#include "Resource.h"

#define ID_TIMER_SIM_CONNECT 100
#define ID_TIMER_POLL_SIM    101

class MainWindow : public winfx::Window, public SimulatorCallbacks {
public:
	MainWindow() : 
		winfx::Window(winfx::loadString(IDC_FLIGHTMONITOR), winfx::loadString(IDS_APP_TITLE)),
		broadcaster_(sim_) {
		sim_.addCallback(this);
		sim_.addCallback(&broadcaster_);
	}

	virtual void modifyWndClass(WNDCLASS& wc) override;
	virtual LRESULT handleWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual winfx::Size getDefaultWindowSize() override {
		return winfx::Size(400, 300);
	}

	LRESULT onCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) override;

	void onSimDataUpdated() override;
	void onSimDisconnect() override;

protected:
	HRESULT connectSim();
	void onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void onDestroy(HWND hwnd);
	void onPaint(HWND hwnd);
	void onTimer(HWND hwnd, UINT idTimer);

private:
	ForeFlightBroadcaster broadcaster_;
	SimulatorInterface sim_;
};

class AboutDialog : public winfx::Dialog {
public:
	AboutDialog(Window* pwnd) : winfx::Dialog(pwnd, IDD_ABOUTBOX) {}
};
