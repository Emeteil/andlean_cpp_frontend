#include "network/osm_tile_texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

OsmTileTexture::OsmTileTexture()
{
}

OsmTileTexture::~OsmTileTexture()
{
    if (_id != 0)
        glDeleteTextures(1, &_id);
}

bool OsmTileTexture::IsFetching() const
{
    return _isFetching;
}

void OsmTileTexture::SetFetching(bool fetching)
{
    std::lock_guard<std::mutex> lock(_texMutex);
    _isFetching = fetching;
}

void OsmTileTexture::StbLoad(const std::vector<std::byte>& rawBlob)
{
    stbi_set_flip_vertically_on_load(false);
    int w, h, c;
    const auto ptr = stbi_load_from_memory(reinterpret_cast<stbi_uc const*>(rawBlob.data()), static_cast<int>(rawBlob.size()), &w, &h, &c, STBI_rgb_alpha);
    
    if (ptr)
    {
        const size_t nbytes = size_t(w * h * STBI_rgb_alpha);
        std::lock_guard<std::mutex> lock(_texMutex);
        _width = w;
        _height = h;
        _channels = c;
        _rgbaBlob.assign(reinterpret_cast<std::byte*>(ptr), reinterpret_cast<std::byte*>(ptr) + nbytes);
        _hasNewData = true;
        _isFetching = false;
        stbi_image_free(ptr);
    }
}

void OsmTileTexture::GlLoad()
{
    std::lock_guard<std::mutex> lock(_texMutex);
    if (!_hasNewData)
        return;
    
    if (_id == 0)
        glGenTextures(1, &_id);

    glBindTexture(GL_TEXTURE_2D, _id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _rgbaBlob.data());
    
    _hasNewData = false;
}

GLuint OsmTileTexture::GetId() const
{
    return _id;
}