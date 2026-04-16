#pragma once

#include "core/data_structures.h"
#include <mutex>

struct UIState
{
    bool showDataWindow = true;
    bool showGraphWindow = true;
    std::string selectedCellIdentity;
};

void DrawMenuBar(UIState& state);
void DrawDataWindow(bool& open, UserData& user, std::mutex& mtx);
void DrawGraphWindow(bool& open, std::vector<SignalData>& signals, std::string& selectedIdentity, std::mutex& mtx);
void DrawMapWindow(bool& open, UserData& currentUser, std::mutex& mtx);