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
