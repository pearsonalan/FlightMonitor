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

class SimulatorInterface {
public:
	SimulatorInterface(Broadcaster* broadcaster, SimulatorCallbacks* callbacks) :
		broadcaster_(broadcaster), callbacks_(callbacks) {}

	HRESULT connectSim(HWND hwnd);
	HRESULT buildDefinition();
	HRESULT pollSimulator();

	void setSimData(const SimData* simData);
	void onSimDisconnect();

	bool isConnected() const { return connected_;  }
	const SimData* getData() const {
		if (has_data_)
			return &data_;
		return nullptr;
	}
	const std::wstring& getConnectResult() const { return connect_result_;  }

private:
	SimulatorCallbacks* callbacks_;
	Broadcaster* broadcaster_;
	bool connected_ = false;
	bool has_data_ = false;
	HANDLE sim_ = INVALID_HANDLE_VALUE;
	long message_ordinal_ = 0;
	std::wstring connect_result_;
	SimData data_;
};

