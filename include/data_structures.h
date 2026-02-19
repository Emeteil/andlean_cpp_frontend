#pragma once

#include <string>
#include <vector>

struct MobileNetworkData {
    std::string networkType;
    std::string cellIdentity;
    std::string mcc;
    std::string mnc;
    int pci;
    int tac;
    std::string bands;
    std::string signalStrength;
    int rsrp;
    int rsrq;
    int rssi;
    int timingAdvance;
    long long time;
};

struct LocationData {
    std::string user;
    double latitude;
    double longitude;
    double altitude;
    long long time;
};

struct UserData {
    std::string user;
    std::vector<MobileNetworkData> mobileNetworks;
    LocationData location;
};

struct SignalData {
    std::string cellIdentity;
    std::vector<double> rsrp_values;
    std::vector<double> rsrq_values;
    std::vector<double> rssi_values;
    std::vector<double> timestamps;
};