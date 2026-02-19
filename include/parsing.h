#pragma once

#include "data_structures.h"
#include <mutex>
#include <vector>

void ParseMobileNetworkData(const std::string& payload, UserData& currentUser, std::mutex& dataMutex, std::vector<SignalData>& signalData);