#include "framework.h"
#include "winfx.h"
#include "Resource.h"

class MainWindow : public winfx::Window {
public:
	MainWindow() : winfx::Window(winfx::loadString(IDC_FLIGHTMONITOR), winfx::loadString(IDS_APP_TITLE)) {}
	virtual void modifyWndClass(WNDCLASS& wc) override;
	virtual LRESULT handleWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual winfx::Size getDefaultWindowSize() override {
		return winfx::Size(400, 300);
	}

protected:
	void onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
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
	}
	return Window::handleWindowMessage(hwndParam, uMsg, wParam, lParam);
}

void MainWindow::onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
	switch (id) {
	case IDM_ABOUT:
		AboutDialog(this).doDialogBox();
		break;

	case IDM_EXIT:
		destroy();
		break;
	}
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
