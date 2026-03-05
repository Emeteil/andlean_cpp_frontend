#include "parsing.h"
#include "json.hpp"
#include <mutex>
#include <algorithm>

using json = nlohmann::json;

void ParseMobileNetworkData(const std::string& payload, UserData& currentUser, std::mutex& dataMutex, std::vector<SignalData>& signalData)
{
    try
    {
        json j = json::parse(payload);
        
        for (auto& [userKey, userData] : j.items())
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            
            currentUser.user = userKey;
            
            if (userData.contains("location_data"))
            {
                auto& loc = userData["location_data"];
                currentUser.location.user = userKey;
                currentUser.location.latitude = loc["Latitude"];
                currentUser.location.longitude = loc["Longitude"];
                currentUser.location.altitude = loc["Altitude"];
                currentUser.location.time = loc["Time"];
            }
            
            if (userData.contains("mobile_network_data_list") && userData["mobile_network_data_list"].contains("MobileNetworks"))
            {
                for (auto& network : userData["mobile_network_data_list"]["MobileNetworks"])
                {
                    MobileNetworkData netData;
                    netData.networkType = network["NetworkType"];
                    netData.cellIdentity = network["CellIdentity"];
                    netData.mcc = network.value("MCC", "");
                    netData.mnc = network.value("MNC", "");
                    netData.pci = network["PCI"];
                    netData.tac = network["TAC"];
                    netData.bands = network["Bands"];
                    netData.signalStrength = network["SignalStrength"];
                    netData.rsrp = network["RSRP"];
                    netData.rsrq = network["RSRQ"];
                    netData.rssi = network["RSSI"];
                    netData.timingAdvance = network["TimingAdvance"];
                    netData.time = network["Time"];

                    auto it = std::find_if(currentUser.mobileNetworks.begin(), currentUser.mobileNetworks.end(),
                        [&](const MobileNetworkData& current) {
                            return current.cellIdentity == netData.cellIdentity;
                        }
                    );

                    if (it != currentUser.mobileNetworks.end())
                        *it = netData;
                    else
                        currentUser.mobileNetworks.push_back(netData);

                    bool signalFound = false;
                    for (auto& signal : signalData)
                    {
                        if (signal.cellIdentity == netData.cellIdentity)
                        {
                            signal.rsrp_values.push_back(netData.rsrp);
                            signal.rsrq_values.push_back(netData.rsrq);
                            signal.rssi_values.push_back(netData.rssi);
                            signal.timestamps.push_back(netData.time);
                            signalFound = true;
                            break;
                        }
                    }

                    if (!signalFound)
                    {
                        SignalData newSignal;
                        newSignal.cellIdentity = netData.cellIdentity;
                        newSignal.rsrp_values.push_back(netData.rsrp);
                        newSignal.rsrq_values.push_back(netData.rsrq);
                        newSignal.rssi_values.push_back(netData.rssi);
                        newSignal.timestamps.push_back(netData.time);
                        signalData.push_back(newSignal);
                    }
                }
            }
        }
    }
    catch (const json::exception& e) {}
}