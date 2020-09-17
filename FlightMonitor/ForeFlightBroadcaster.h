#pragma once

#include "framework.h"
#include "winfx.h"
#include "SimData.h"
#include "BroadcasterInterface.h"

#define FF_GPS_PORT       49002

class ForeFlightBroadcaster : public Broadcaster {
public:
	static int InitWinsock();

	HRESULT init();

	BOOL broadcastPositionReport(const SimData* data) override;
	BOOL broadcastAttitudeReport(const SimData* data) override;

private:
	SOCKET sock_ = INVALID_SOCKET;
	sockaddr_in send_addr_ = { 0 };
};
