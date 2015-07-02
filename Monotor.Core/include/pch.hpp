#pragma once

#include "targetver.hpp"

#include <cassert>

#include <algorithm>
#include <numeric>
#include <memory>
#include <vector>
#include <unordered_map>

#include <winapifamily.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wrl\client.h>
#include <dshow.h>
#include <evr.h>