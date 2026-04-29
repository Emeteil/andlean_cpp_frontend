#pragma once

#include "core/data_structures.h"
#include <mutex>
#include <vector>
#include <atomic>

void FetchDataFromDBThread(UserData* currentUser, std::mutex* dataMutex, std::atomic<bool>* is_loading);