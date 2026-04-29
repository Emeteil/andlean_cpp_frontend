#include "ui/ui_heatmap.h"
#include "core/color_funcs.h"
#include "core/mercator_projection.h"
#include <implot.h>
#include <algorithm>
#include <thread>
#include <iostream>
#include <cmath>

#define USE_GAUSSIAN 1
#define USE_IDW 0

struct Dot { double x, y; int rsrp; };
struct DotW { double distSqrt; Dot p; };

#if USE_GAUSSIAN
    struct InterpResult { double rsrp; double minDistSq; };
    InterpResult InterpolateGaussian(double x, double y, const std::vector<Dot>& dots)
    {
        double sigma = 0.01;
        double sigma2 = sigma * sigma;
        double cutoffSq = (3.0 * sigma) * (3.0 * sigma);
        double wgt = 0.0, result = 0.0;
        double minDistSq = 999999999.0;

        for (const auto& dot : dots)
        {
            double dx = dot.x - x;
            double dy = dot.y - y;
            double d2 = dx*dx + dy*dy;
            if (d2 < minDistSq)
                minDistSq = d2;
            
            if (d2 > cutoffSq)
                continue;
            double w = std::exp(-d2 / (2.0 * sigma2));
            wgt += w;
            result += w * dot.rsrp;
        }

        return { result / wgt, minDistSq };
    }
#else
    // insert sort for 3 elements
    void Find3NearestDots(double x, double y, const std::vector<Dot>& dots, DotW outDots[3])
    {
        outDots[0] = outDots[1] = outDots[2] = { 999999.0, {0, 0, 0} };

        for (const auto& dot : dots)
        {
            double distSqrt = (dot.x-x) * (dot.x-x) + (dot.y-y) * (dot.y- y);

            if (distSqrt < outDots[2].distSqrt)
            {
                outDots[2] = { distSqrt, dot };
                if (outDots[2].distSqrt < outDots[1].distSqrt) std::swap(outDots[2], outDots[1]);
                if (outDots[1].distSqrt < outDots[0].distSqrt) std::swap(outDots[1], outDots[0]);
            }
        }
    }

    // интерполяция по обратному средневзвешенному расстоянию (idw)
    double InterpolateIDW3(double x, double y, const DotW dotsW[3])
    {
        // Source - https://stackoverflow.com/a/6279144
        // Posted by Tebas
        // Retrieved 2026-04-29, License - CC BY-SA 3.0
        double wgt = 0, wgt_tmp, result = 0;
        for (int i = 0; i < 3; i++){
            if (dotsW[i].distSqrt < 1e-10) return dotsW[i].p.rsrp;
            wgt_tmp = 1.0/dotsW[i].distSqrt;
            wgt += wgt_tmp;
            result += wgt_tmp * dotsW[i].p.rsrp;
        }
        return result / wgt;
    }

    double LinearInterpolate3(double x, double y, const DotW dotsW[3])
    {
        double z1 = dotsW[0].p.rsrp; double w1 = 1.0f/dotsW[0].distSqrt;
        double z2 = dotsW[1].p.rsrp; double w2 = 1.0f/dotsW[1].distSqrt;
        double z3 = dotsW[2].p.rsrp; double w3 = 1.0f/dotsW[2].distSqrt;
        return (z1 * w1 + z2 * w2 + z3 * w3) / (w1 + w2 + w3);
    }
#endif


ImVec4 RsrpToColorImVec4(int rsrp)
{
    float minRsrp = -120.0f;
    float maxRsrp = -70.0f;

    float t = (static_cast<float>(rsrp) - minRsrp) / (maxRsrp - minRsrp);
    t = std::max(0.0f, std::min(1.0f, t));

    RGB rgb = hsl2rgb(t * 0.333f, 1.0f, 0.5f);

    return ImVec4(rgb.r / 255.0f, rgb.g / 255.0f, rgb.b / 255.0f, 0.3f);
}

HeatmapRenderer::~HeatmapRenderer()
{
    if (_textureId != 0) glDeleteTextures(1, &_textureId);
}

void HeatmapRenderer::UpdateAndDraw(const std::vector<MapPoint>& mapPoints)
{
    if (_lastPointsSize != mapPoints.size() && !_isCalculating)
    {
        _lastPointsSize = mapPoints.size();
        _isCalculating = true;

        std::vector<MapPoint> pointsCopy = mapPoints;

        std::thread([this, pointsCopy]()
        {
            if (pointsCopy.empty())
            {
                _isCalculating = false;
                return;
            }

            double minX = 999999;
            double maxX = -999999;
            double minY = 999999;
            double maxY = -999999;

            std::vector<Dot> dots;
            dots.reserve(pointsCopy.size());
            for (const auto& point : pointsCopy)
            {
                double x = MercatorProjection::ToEPSG3857_X(point.longitude);
                double y = MercatorProjection::ToEPSG3857_Y(point.latitude);
                dots.push_back({x, y, point.rsrp});

                if (x < minX)
                    minX = x - 0.03;
                if (x > maxX)
                    maxX = x + 0.03;
                if (y < minY)
                    minY = y - 0.03;
                if (y > maxY)
                    maxY = y + 0.03;
            }

            int width = 256;
            int height = 256;
            std::vector<std::byte> pixels(width * height * 4);

            for (int py = 0; py < height; py++)
            {
                for (int px = 0; px < width; px++)
                {
                    double x = minX + (maxX - minX) * (double)px / (width - 1);
                    double y = maxY + (minY - maxY) * (double)py / (height - 1);

                    #if USE_GAUSSIAN
                        auto [interpolatedRsrp, minDistSq] = InterpolateGaussian(x, y, dots);
                        ImVec4 color = RsrpToColorImVec4(static_cast<int>(interpolatedRsrp));
                        double alphaFactor = (minDistSq >= 0.0004) ? 0.0 : 1.0;
                    #else
                        DotW dotsW[3];
                        Find3NearestDots(x, y, dots, dotsW);

                        double interpolatedRsrp;
                        #if USE_IDW
                            interpolatedRsrp = InterpolateIDW3(x, y, dotsW);
                        #else
                            interpolatedRsrp = LinearInterpolate3(x, y, dotsW);
                        #endif

                        ImVec4 color = RsrpToColorImVec4(static_cast<int>(interpolatedRsrp));
                        double alphaFactor = (dotsW[0].distSqrt >= 0.0004f) ? 0.0f : 1.0f;
                    #endif

                    int idx = (py * width + px) * 4;
                    pixels[idx + 0] = static_cast<std::byte>(std::clamp(color.x * 255.0f, 0.0f, 255.0f));
                    pixels[idx + 1] = static_cast<std::byte>(std::clamp(color.y * 255.0f, 0.0f, 255.0f));
                    pixels[idx + 2] = static_cast<std::byte>(std::clamp(color.z * 255.0f, 0.0f, 255.0f));
                    pixels[idx + 3] = static_cast<std::byte>(std::clamp(static_cast<float>(color.w * 255.0f * 1.5f * alphaFactor), 0.0f, 255.0f));
                }
            }

            _width = width;
            _height = height;
            _minX = minX;
            _maxX = maxX;
            _minY = minY;
            _maxY = maxY;
            _pixels = std::move(pixels);
            
            _hasNewData = true;
            _isCalculating = false;
        }).detach();
    }

    if (_hasNewData)
    {
        if (_textureId == 0) glGenTextures(1, &_textureId);
        glBindTexture(GL_TEXTURE_2D, _textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels.data());
        _hasNewData = false;
    }

    if (_textureId != 0 && !_isCalculating && !_hasNewData)
    {
        ImPlotPoint bmin{_minX, _minY};
        ImPlotPoint bmax{_maxX, _maxY};
        ImVec2 uv0{0, 0};
        ImVec2 uv1{1, 1};
        ImPlot::PlotImage("Heatmap", (ImTextureID)(intptr_t)_textureId, bmin, bmax, uv0, uv1);
    }
}