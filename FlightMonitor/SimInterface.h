#pragma once

#include "framework.h"
#include "winfx.h"
#include "SimData.h"
#include "BroadcasterInterface.h"

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
	SimulatorInterface(Broadcaster* broadcaster, SimulatorCallbacks* callbacks) :
		broadcaster_(broadcaster), callbacks_(callbacks) {}

	HRESULT connectSim(HWND hwnd);
	HRESULT pollSimulator();
	void close();

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

	SimulatorCallbacks* callbacks_;
	Broadcaster* broadcaster_;
	HANDLE sim_ = INVALID_HANDLE_VALUE;
	long message_ordinal_ = 0;
	SimulatorInterfaceState state_ = SimInterfaceDisconnected;
	SimData data_;
};

