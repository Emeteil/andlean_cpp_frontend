#pragma once

#include <vector>
#include <functional>
#include <cstddef>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

struct OsmTileCoord
{
    int zoom;
    int x;
    int y;
};

class OsmTileFetcher
{
    public:
        OsmTileFetcher(size_t threads = 2);
        ~OsmTileFetcher();
        void ClearJobs();
        void Fetch(const std::vector<OsmTileCoord>& coords, std::function<void(const OsmTileCoord&, const std::vector<std::byte>&)> callback);

    private:
        struct Job
        {
            OsmTileCoord coord;
            std::function<void(const OsmTileCoord&, const std::vector<std::byte>&)> callback;
        };

        std::vector<std::thread> workers;
        std::queue<Job> jobs;
        std::mutex queueMutex;
        std::condition_variable condition;
        bool stopPool;

        void WorkerLoop();
        void ProcessJob(const Job& job);
        static size_t OnPullResponse(void* data, size_t size, size_t nmemb, void* userp);
};