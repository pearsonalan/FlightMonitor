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

class SimulatorCallbacks {
public:
	virtual void onSimDataUpdated() = 0;
	virtual void onSimDisconnect() = 0;
};

enum SimulatorInterfaceState {
	SimInterfaceDisconnected = 0,
	SimInterfaceConnected,
	SimInterfaceReceivingData,
	SimInterfaceInFlight
};

class SimulatorInterface {
public:
	HRESULT connectSim(HWND hwnd);
	HRESULT pollSimulator();
	void close();
	void addCallback(SimulatorCallbacks* callback) {
		callbacks_.push_back(callback);
	}

	bool isConnected() const { return state_ != SimInterfaceDisconnected;  }
	const SimData* getData() const {
		if (state_ == SimInterfaceReceivingData || state_ == SimInterfaceInFlight)
			return &data_;
		return nullptr;
	}
	SimulatorInterfaceState getState() const { return state_;  }
	const std::wstring& getStatusMessage() const;

	// Callbacks from the SimConnect dispatch proc
	void setSimData(const SimData* simData);
	void onSimDisconnect();

private:
	bool positionIsValid();
	HRESULT buildDefinition();

	std::vector<SimulatorCallbacks*> callbacks_;
	HANDLE sim_ = INVALID_HANDLE_VALUE;
	SimulatorInterfaceState state_ = SimInterfaceDisconnected;
	SimData data_;
};

