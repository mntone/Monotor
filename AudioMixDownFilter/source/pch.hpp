#pragma once

#include "targetver.h"

#include <cstdint>
#include <cassert>

#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <mutex>

#include <winapifamily.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wrl\client.h>
#include <dshow.h>
#include <dvdmedia.h>

#include "Utilties.hpp"