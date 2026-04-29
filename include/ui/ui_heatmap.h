#pragma once

#include <vector>
#include <atomic>
#include <cstddef>
#include <GL/glew.h>
#include <imgui.h>
#include "core/data_structures.h"

ImVec4 RsrpToColorImVec4(int rsrp);

class HeatmapRenderer
{
    public:
        HeatmapRenderer() = default;
        ~HeatmapRenderer();

        void UpdateAndDraw(const std::vector<MapPoint>& mapPoints);

    private:
        std::vector<std::byte> _pixels;
        double _minX = 0;
        double _maxX = 0;
        double _minY = 0;
        double _maxY = 0;
        int _width = 0;
        int _height = 0;
        GLuint _textureId = 0;
        size_t _lastPointsSize = static_cast<size_t>(-1);
        std::atomic<bool> _isCalculating{false};
        std::atomic<bool> _hasNewData{false};
};