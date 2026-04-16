#pragma once

#include <vector>
#include <cstddef>
#include <mutex>
#include <GL/glew.h>

class OsmTileTexture
{
    public:
        OsmTileTexture();
        ~OsmTileTexture();

        void StbLoad(const std::vector<std::byte>& rawBlob);
        void GlLoad();
        
        GLuint GetId() const;
        bool IsFetching() const;
        void SetFetching(bool fetching);

    private:
        int _width{256};
        int _height{256};
        int _channels{};
        std::vector<std::byte> _rgbaBlob;
        GLuint _id{0};
        bool _hasNewData{false};
        bool _isFetching{false};
        std::mutex _texMutex;
};