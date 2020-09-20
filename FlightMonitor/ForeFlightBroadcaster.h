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
#include "SimData.h"
#include "SimInterface.h"

// Implement ForeFlight GPS Integration as documented at
//   https://support.foreflight.com/hc/en-us/articles/204115005-Flight-Simulator-GPS-Integration-UDP-Protocol-
//

#define FF_GPS_PORT       49002

constexpr int kAttitueReportsPerSecond = 5;

class ForeFlightBroadcaster : public SimulatorCallbacks {
public:
	ForeFlightBroadcaster(const SimulatorInterface& sim) : sim_(sim) {}

	static HRESULT InitWinsock();

	HRESULT init();
	void onSimDataUpdated() override;
	void onSimDisconnect() override;

private:
	BOOL broadcastPositionReport(const SimData* data);
	BOOL broadcastAttitudeReport(const SimData* data);

	SOCKET sock_ = INVALID_SOCKET;
	sockaddr_in send_addr_ = { 0 };
	const SimulatorInterface& sim_;
	long message_ordinal_ = 0;
};
