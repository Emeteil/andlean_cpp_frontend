#include "ui_components.h"
#include "mercator_projection.h"
#include <imgui.h>
#include <implot.h>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <memory>
#include <cmath>
#include "osm_tile_fetcher.h"
#include "osm_tile_texture.h"
#include "color_funcs.h"
#include <iostream>

ImVec4 RsrpToColorImVec4(int rsrp)
{
    float minRsrp = -120.0f;
    float maxRsrp = -70.0f;

    float t = (static_cast<float>(rsrp) - minRsrp) / (maxRsrp - minRsrp);
    t = std::max(0.0f, std::min(1.0f, t));

    RGB rgb = hsl2rgb(t * 0.333f, 1.0f, 0.5f);

    return ImVec4(rgb.r / 255.0f, rgb.g / 255.0f, rgb.b / 255.0f, 0.3f);
}

void DrawMapWindow(bool &open, UserData &currentUser, std::mutex &mtx)
{
    if (!open) return;

    if (ImGui::Begin("Map", &open, ImGuiWindowFlags_MenuBar))
    {
        std::lock_guard<std::mutex> lock(mtx);

        static bool init = false;
        if (!init)
        {
            double x = MercatorProjection::ToEPSG3857_X(82.939999);
            double y = MercatorProjection::ToEPSG3857_Y(54.981952);
            ImPlot::SetNextAxesLimits(x - 0.1, x + 0.1, y - 0.1, y + 0.1, ImPlotCond_Once);
            init = true;
        }

        if (ImPlot::BeginPlot("##ImOsmMapPlot", ImVec2(-1, -1), ImPlotFlags_NoLegend | ImPlotFlags_Equal))
        {
            static std::map<std::string, OsmTileTexture> tileCache;
            static OsmTileFetcher fetcher(25);
            static ImPlotRect lastLimits;

            ImPlotRect limits = ImPlot::GetPlotLimits();

            if (limits.X.Min != lastLimits.X.Min || limits.X.Max != lastLimits.X.Max || limits.Y.Min != lastLimits.Y.Min || limits.Y.Max != lastLimits.Y.Max)
            {
                fetcher.ClearJobs();
                for (auto& entry : tileCache)
                    entry.second.SetFetching(false);
                lastLimits = limits;
            }

            int zoom = std::clamp(static_cast<int>(-std::log(limits.X.Max - limits.X.Min) * 1.3 + 12), 4, 18);

            static int lastZoom = zoom;
            if (zoom != lastZoom)
            {
                std::cout << "Zoom: " << zoom << std::endl;
                fetcher.ClearJobs();
                for (auto& entry : tileCache)
                    entry.second.SetFetching(false);
                lastZoom = zoom;
            }

            int minX = MercatorProjection::ConvertEPSG3857ToTileX(limits.X.Min, zoom);
            int maxX = MercatorProjection::ConvertEPSG3857ToTileX(limits.X.Max, zoom);
            int minY = MercatorProjection::ConvertEPSG3857ToTileY(limits.Y.Max, zoom);
            int maxY = MercatorProjection::ConvertEPSG3857ToTileY(limits.Y.Min, zoom);

            if (limits.X.Max - limits.X.Min < 1200)
            for (int tileX = minX; tileX <= maxX; tileX++)
            {
                for (int tileY = minY; tileY <= maxY; tileY++)
                {
                    std::string tileId = std::to_string(zoom) + "/" + std::to_string(tileX) + "/" + std::to_string(tileY);
                    auto it = tileCache.find(tileId);
                    bool needsRequest = (it == tileCache.end()) || (it->second.GetId() == 0 && !it->second.IsFetching());

                    if (needsRequest)
                    {
                        OsmTileTexture& texture = tileCache[tileId];
                        texture.SetFetching(true);

                        int count = 1 << zoom;
                        int reqTileX = tileX % count;
                        int reqTileY = tileY % count;
                        if (reqTileX < 0) reqTileX += count;
                        if (reqTileY < 0) reqTileY += count;

                        fetcher.Fetch({{zoom, reqTileX, reqTileY}}, [&texture](const OsmTileCoord& c, const std::vector<std::byte>& blob) {
                            texture.StbLoad(blob);
                        });
                    }

                    auto& texture = tileCache[tileId];
                    texture.GlLoad();

                    if (texture.GetId() != 0)
                    {
                        ImVec2 uv0{0, 0};
                        ImVec2 uv1{1, 1};
                        ImVec4 tint{1, 1, 1, 1};
                        ImPlotPoint bmin{MercatorProjection::TileXToEPSG3857(tileX, zoom), MercatorProjection::TileYToEPSG3857(tileY + 1, zoom)};
                        ImPlotPoint bmax{MercatorProjection::TileXToEPSG3857(tileX + 1, zoom), MercatorProjection::TileYToEPSG3857(tileY, zoom)};
                        ImPlot::PlotImage(("##tile_" + tileId).c_str(), (ImTextureID)(intptr_t)texture.GetId(), bmin, bmax, uv0, uv1, tint);
                    }
                }
            }

            for (size_t i = 0; i < currentUser.mapPoints.size(); i++)
            {
                const auto &point = currentUser.mapPoints[i];
                double x = MercatorProjection::ToEPSG3857_X(point.longitude);
                double y = MercatorProjection::ToEPSG3857_Y(point.latitude);
                ImVec4 fillColor = RsrpToColorImVec4(point.rsrp);

                ImPlotSpec spec;
                spec.Marker = ImPlotMarker_Circle;
                spec.MarkerSize = 6.5f;
                spec.MarkerFillColor = fillColor;
                spec.MarkerLineColor = fillColor;

                std::string label = "Point_" + std::to_string(i);
                ImPlot::PlotScatter(label.c_str(), &x, &y, 1, spec);
            }

            ImPlot::EndPlot();
        }
    }
    ImGui::End();
}