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
#include <utility>

#include "Chozo/Debug/Log.h"

#ifndef WINDOW_WIDTH
#define WINDOW_WIDTH 1920
#endif

#ifndef WINDOW_HEIGHT
#define WINDOW_HEIGHT 1080
#endif

#ifdef CHOZO_PLATFORM_WIN
    #include <Windows.h>
#endif