#include "network/db_fetcher.h"
#include <curl/curl.h>
#include "json.hpp"
#include "core/parsing.h"
#include <string>
#include <iostream>

static size_t WriteCallbackMap(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void FetchDataFromDBThread(UserData* currentUser, std::mutex* dataMutex, std::atomic<bool>* is_loading)
{
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        *is_loading = false;
        return;
    }

    int page = 1;
    while (true)
    {
        std::string readBuffer;
        std::string url = "http://94.159.111.243:5678/api/mobile-network/data?count=4000&page=" + std::to_string(page);
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackMap);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) break;

        try
        {
            auto j = nlohmann::json::parse(readBuffer);
            if (j.contains("data") && j["data"].contains("data"))
            {
                auto& dataObj = j["data"]["data"];
                if (dataObj.empty()) break;
                
                std::vector<MapPoint> newPoints;
                
                auto processItem = [&](const nlohmann::json& item) {
                    if (item.contains("location_data") && !item["location_data"].is_null() && 
                        item.contains("mobile_network_data_list") && !item["mobile_network_data_list"].is_null() && 
                        item["mobile_network_data_list"].contains("MobileNetworks"))
                    {
                        auto& loc = item["location_data"];
                        MapPoint point;
                        point.latitude = loc.value("Latitude", 0.0);
                        point.longitude = loc.value("Longitude", 0.0);
                        
                        int maxRSRP = -1000;
                        for (auto& network : item["mobile_network_data_list"]["MobileNetworks"]) {
                            if (network.contains("RSRP") && !network["RSRP"].is_null()) {
                                int rsrp = network["RSRP"];
                                if (rsrp > maxRSRP) maxRSRP = rsrp;
                            }
                        }
                        point.rsrp = maxRSRP;
                        newPoints.push_back(point);
                    }
                };

                if (dataObj.is_array())
                {
                    for (auto& item : dataObj)
                    {
                        processItem(item);
                    }
                }
                else if (dataObj.is_object())
                {
                    for (auto& [userKey, userArray] : dataObj.items())
                    {
                        if (userArray.is_array())
                        {
                            for (auto& item : userArray)
                            {
                                processItem(item);
                            }
                        }
                    }
                }
                
                {
                    std::lock_guard<std::mutex> lock(*dataMutex);
                    currentUser->mapPoints.insert(currentUser->mapPoints.end(), newPoints.begin(), newPoints.end());
                }
                std::cout << "Fetched page " << page << ", size: " << currentUser->mapPoints.size() << std::endl;
            }
            else
            {
                break;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Exception on page " << page << ": " << e.what() << std::endl;
            break;
        }
        catch (...)
        {
            std::cerr << "Unknown exception on page " << page << std::endl;
            break;
        }
        page++;
    }
    
    curl_easy_cleanup(curl);
    *is_loading = false;
}