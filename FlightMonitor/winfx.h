#ifndef _winfx_h_
#define _winfx_h_

namespace winfx {

struct Point : public tagPOINT {
	Point(LONG x_,LONG y_) { x = x_; y = y_; }
};
	
struct Size : public tagSIZE {
	Size(LONG cx_,LONG cy_) { cx = cx_; cy = cy_; }
};

struct Rect : public tagRECT {
	Rect() { top = bottom = right = left = 0; }
	Rect(LONG l, LONG t, LONG r, LONG b) {
		left = l;
		top = t;
		right = r;
		bottom = b;
	}
	int height() { return bottom - top; }
	int width() { return right - left; }
	operator LPRECT () { return this; }
	operator LPCRECT () const { this; }
};
	
class App {
protected:
	HINSTANCE 	hInst;
	DWORD		dwExitCode ;

	static App* singleton;
		
public:

	App() {
		hInst = 0;
		dwExitCode = 0;
		singleton = this;
	}

	virtual ~App();

	static App& getSingleton() { return *singleton; }
		
	HINSTANCE getInstance() { return hInst; }
	DWORD getExitCode() { return dwExitCode; }
	std::wstring loadString(UINT uID) {
		wchar_t buffer[1024];
		LoadStringW(hInst, uID, buffer, 1024);
		return std::wstring(buffer);
	}

	virtual bool initInstance(HINSTANCE hInst, HINSTANCE hInstPrev);
	virtual bool initWindow(LPWSTR pwstrCmdLine, int nCmdShow) = 0;
	virtual void terminate();
	virtual bool translateModelessMessage(MSG* pmsg);
};

class Window {
protected:
	HWND			hwnd;
	Window*			pwndParent;
	std::wstring	className;
	std::wstring	windowName;
	bool			classIsRegistered;

public:
	
	Window(Window* pwndParent_in = 0) :
		hwnd(0),
		pwndParent(pwndParent_in),
		classIsRegistered(false)
		{		
		}

	Window(std::wstring classNameIn, std::wstring windowNameIn, Window* pwndParentIn = 0) :
		hwnd(0),
		pwndParent(pwndParentIn),
		className(classNameIn),
		windowName(windowNameIn),
		classIsRegistered(false)
		{		
		}

	virtual ~Window();
	
	HWND getWindow() { return hwnd; }
	virtual bool create(LPWSTR pstrCmdLine, int nCmdShow);
	void destroy() { ::DestroyWindow(hwnd); }
	void setWindowHandle(HWND hwndParam) { hwnd = hwndParam; }

	Rect getClientRect() { Rect r; ::GetClientRect(hwnd, (LPRECT)r); return r; }
		
	int messageBox(const std::wstring& text, const std::wstring& caption, UINT uType = MB_OK ) {
		return ::MessageBox(hwnd, text.c_str(), caption.c_str(), uType);
	}
	void showWindow(int nCmdShow) { ::ShowWindow(hwnd, nCmdShow); }
	bool setWindowPos(HWND hwndBefore, int x, int y, int cx, int cy, UINT uiFlags) {
		return ::SetWindowPos(hwnd, hwndBefore, x, y, cx, cy, uiFlags); 
	}
	bool postMessage(UINT msg, WPARAM wparam, LPARAM lparam) { 
		return (bool) ::PostMessage(hwnd, msg, wparam, lparam);
	}
		
	virtual LRESULT onClose(HWND hwnd);
	virtual LRESULT onCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
	
	bool registerWindowClass();
	virtual Point getDefaultWindowPosition();
	virtual Size getDefaultWindowSize();
		
	virtual void modifyWndClass(WNDCLASS& wc);
	virtual LRESULT handleWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class Dialog : public Window {
protected:
	int idd;

	friend BOOL CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:

	Dialog(Window* pwnd, int idd_in) :
		Window(pwnd),
		idd(idd_in)
		{
		}

	HWND getDlgItem(int id) { return GetDlgItem( hwnd, id ); }
	LRESULT sendDlgItemMessage(int id, UINT msg, WPARAM wparam, LPARAM lparam) { 
		return SendDlgItemMessage(hwnd, id, msg, wparam, lparam); 
	}
		
	bool endDialog(int nResult);
		
	std::wstring getItemText(int id);
	void setItemText(int id, const std::wstring& str);
	
	virtual LRESULT onInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	virtual LRESULT onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)	;
	virtual LRESULT handleWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	void create();
	int doDialogBox();
};

inline bool Dialog::endDialog(int nResult) {
	return EndDialog(hwnd, nResult);
}

inline BOOL textOut(HDC hdc, int x, int y, const std::wstring& str) {
	return TextOutW(hdc, x, y, str.c_str(), static_cast<int>(str.size()));
}

inline std::wstring loadString(UINT uID) {
	return App::getSingleton().loadString(uID);
}

void DebugOut(LPCWSTR format ...);

}  // namespace winfx

#endif
