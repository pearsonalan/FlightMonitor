#include "framework.h"
#include "winfx.h"
#include "ForeFlightBroadcaster.h"

int ForeFlightBroadcaster::InitWinsock() {
	WORD wVersionRequested = MAKEWORD(2, 2);

	WSADATA wsaData;
	int err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		// Tell the user that we could not find a usable Winsock DLL.
		winfx::DebugOut(L"WSAStartup failed with error: %d\n", err);
		return 1;
	}

	return 0;
}

HRESULT ForeFlightBroadcaster::init() {
	sock_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_ == INVALID_SOCKET) {
		winfx::DebugOut(L"Error %d allocating socket\n", WSAGetLastError());
		return E_FAIL;
	}

	char broadcast = '1';
	if (setsockopt(sock_, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
		winfx::DebugOut(L"Error %d setting socket broadcast option\n", WSAGetLastError());
		closesocket(sock_);
		sock_ = INVALID_SOCKET;
		return E_FAIL;
	}

	send_addr_.sin_family = AF_INET;
	send_addr_.sin_port = htons(FF_GPS_PORT);

	// TODO: get correct broadcast address
	
	// Could hardcode an address with a net mask...
	// inet_pton(AF_INET, "192.168.86.255", &send_addr_.sin_addr.s_addr);

	// ... or get addresses and masks with GetAddresses() and GetAdapterInfo()

	// ... or use INADDR_BROADCAST
	send_addr_.sin_addr.s_addr = INADDR_BROADCAST;

	return S_OK;
}

const char* SIM_NAME = "MSFS";

BOOL ForeFlightBroadcaster::broadcastPositionReport(const SimData* data) {
	if (sock_ == INVALID_SOCKET) {
		winfx::DebugOut(L"Cannot send position report. Socket invalid.\n");
		return FALSE;
	}

	char send_buffer[256] = { 0 };
	sprintf_s(send_buffer, "XGPS%s,%0.4f,%0.4f,%0.1f,%0.2f,%01.f",
		SIM_NAME, data->gps_lon, data->gps_lat, data->gps_alt, data->gps_track, data->gps_groundspeed);
	winfx::DebugOut(L"GPS Message: %S\n", send_buffer);
	if (sendto(sock_, send_buffer, (int)strlen(send_buffer), 0, (sockaddr*)&send_addr_,
		(int)sizeof(send_addr_)) == SOCKET_ERROR) {
		int err = WSAGetLastError();
		winfx::DebugOut(L"Error %d in send.\n", err);
		return FALSE;
	}

	return TRUE;
}

BOOL ForeFlightBroadcaster::broadcastAttitudeReport(const SimData* data) {
	if (sock_ == INVALID_SOCKET) {
		winfx::DebugOut(L"Cannot send position report. Socket invalid.\n");
		return FALSE;
	}

	char send_buffer[256] = { 0 };
	sprintf_s(send_buffer, "XATT%s,%0.4f,%0.4f,%0.4f",
		SIM_NAME, data->heading, -data->pitch, data->bank);
	winfx::DebugOut(L"ATT Message: %S\n", send_buffer);
	if (sendto(sock_, send_buffer, (int)strlen(send_buffer), 0, (sockaddr*)&send_addr_,
		(int)sizeof(send_addr_)) == SOCKET_ERROR) {
		int err = WSAGetLastError();
		winfx::DebugOut(L"Error %d in send.\n", err);
		return FALSE;
	}

	return TRUE;
}
