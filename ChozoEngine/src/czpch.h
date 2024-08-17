#pragma once

#include <iostream>
#include <memory>
#include <algorithm>
#include <functional>

#include <stdint.h>
#include <string>
#include <string_view>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Chozo/Debug/Log.h"

#ifndef WINDOW_WIDTH
#define WINDOW_WIDTH 1600
#endif

#ifndef WINDOW_HEIGHT
#define WINDOW_HEIGHT 900
#endif

#ifdef CHOZO_PLATFORM_WIN
    #include <Windows.h>
#endif