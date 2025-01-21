#pragma once

#include <iostream>

#include <cassert>
#include <sstream>
#include <type_traits>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#else
// #error "Only windows is supported for now"
#endif

#include <vector>
#include <set>
#include <queue>
#include <stack>
#include <unordered_map>
#include <shared_mutex>
#include <thread>
#include <functional>
#include <format>
#include <filesystem>
#include <fstream>
#include <string_view>

#include <numbers>
#include <random>
#include <cmath>

#include <chrono>
#include <time.h>
#include <iomanip>