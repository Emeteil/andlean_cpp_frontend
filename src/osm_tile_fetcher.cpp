#include "osm_tile_fetcher.h"
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <sstream>

OsmTileFetcher::OsmTileFetcher(size_t threads) : stopPool(false)
{
    for (size_t i = 0; i < threads; ++i)
    {
        workers.push_back(std::thread(&OsmTileFetcher::WorkerLoop, this));
    }
}

OsmTileFetcher::~OsmTileFetcher()
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stopPool = true;
    }
    condition.notify_all();

    for (std::thread& worker : workers)
    {
        if (worker.joinable())
            worker.join();
    }
    workers.clear();
}
 
void OsmTileFetcher::ClearJobs()
{
    std::unique_lock<std::mutex> lock(queueMutex);
    while (!jobs.empty())
        jobs.pop();
}

size_t OsmTileFetcher::OnPullResponse(void* data, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    auto& blob = *static_cast<std::vector<std::byte>*>(userp);
    auto const* const dataptr = static_cast<std::byte*>(data);
    blob.insert(blob.cend(), dataptr, dataptr + realsize);
    return realsize;
}

void OsmTileFetcher::ProcessJob(const Job& job)
{
    std::vector<std::byte> blob;
    std::stringstream pathBuilder;
    pathBuilder << "cache/" << job.coord.zoom << "/" << job.coord.x;
    std::filesystem::path dir(pathBuilder.str());
    std::filesystem::path filePath = dir / (std::to_string(job.coord.y) + ".png");

    if (std::filesystem::exists(filePath))
    {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (file.is_open())
        {
            auto size = file.tellg();
            file.seekg(0, std::ios::beg);
            blob.resize(size);
            file.read(reinterpret_cast<char*>(blob.data()), size);
            job.callback(job.coord, blob);
            return;
        }
    }

    CURL* curl = curl_easy_init();
    if (!curl)
        return;

    std::ostringstream urlmaker;
    urlmaker << "https://tile.openstreetmap.org/" << job.coord.zoom << '/' << job.coord.x << '/' << job.coord.y << ".png";

    std::string url = urlmaker.str();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "andlean_cpp_frontend/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&blob);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnPullResponse);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK && !blob.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        std::ofstream file(filePath, std::ios::binary);

        if (file.is_open())
            file.write(reinterpret_cast<const char*>(blob.data()), blob.size());
        
        job.callback(job.coord, blob);
    }
}

void OsmTileFetcher::WorkerLoop()
{
    while (true)
    {
        Job job;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stopPool || !jobs.empty(); });

            if (stopPool)
                return;

            job = jobs.front();
            jobs.pop();
        }
        ProcessJob(job);
    }
}

void OsmTileFetcher::Fetch(const std::vector<OsmTileCoord>& coords, std::function<void(const OsmTileCoord&, const std::vector<std::byte>&)> callback)
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        for (const auto& coord : coords)
            jobs.push({coord, callback});
    }
    condition.notify_all();
}