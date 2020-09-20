// framework.h : include file for standard system include files,
// or project specific include files
//
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

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Windows Header Files
#include <windows.h>
#include <windowsx.h>
#include <winsock2.h>
#include <WS2tcpip.h>

// MSFS SimConnect Header Files
#include <SimConnect.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// C++ Header Files
#include <string>
#include <map>
#include <vector>
