#pragma once

#include "framework.h"
#include "winfx.h"
#include "SimData.h"

class Broadcaster {
public:
	virtual BOOL broadcastPositionReport(const SimData* data) = 0;
	virtual BOOL broadcastAttitudeReport(const SimData* data) = 0;
};