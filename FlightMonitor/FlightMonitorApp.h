#pragma once

#include "framework.h"
#include "winfx.h"
#include "MainWindow.h"

class FlightMonitorApp : public winfx::App {
protected:
	MainWindow mainWindow;
public:
	virtual bool initWindow(LPWSTR pwstrCmdLine, int nCmdShow);
};