#include "ui_widget.h"
#include "ui_components.h"
#include <imgui.h>

void CreateMobileNetworkWidget(UserData& currentUser, std::mutex& dataMutex, std::vector<SignalData>& signalData)
{
    static UIState state;

    DrawMenuBar(state);
    DrawDataWindow(state.showDataWindow, currentUser, dataMutex);
    DrawGraphWindow(state.showGraphWindow, signalData, state.selectedCellIdentity, dataMutex);
}