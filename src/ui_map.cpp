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

ImVec4 RsrpToColorImVec4(int rsrp)
{
    ImColor color = IM_COL32(0, 255, 0, 255);

    return ImVec4(
        ((color >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f,
        ((color >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f,
        ((color >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f,
        ((color >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f);
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

        if (ImPlot::BeginPlot("##ImOsmMapPlot", ImVec2(-1, 0), ImPlotFlags_NoLegend | ImPlotFlags_Equal))
        {
            static std::map<std::string, OsmTileTexture> tileCache;
            static OsmTileFetcher fetcher(30);
            static ImPlotRect lastLimits;

            ImPlotRect limits = ImPlot::GetPlotLimits();

            if (limits.X.Min != lastLimits.X.Min || limits.X.Max != lastLimits.X.Max || limits.Y.Min != lastLimits.Y.Min || limits.Y.Max != lastLimits.Y.Max)
            {
                fetcher.ClearJobs();
                for (auto& entry : tileCache)
                    entry.second.SetFetching(false);
                lastLimits = limits;
            }

            int zoom;
            if (limits.X.Max - limits.X.Min < 0.01)
                zoom = 18;
            else if (limits.X.Max - limits.X.Min < 0.02)
                zoom = 17;
            else if (limits.X.Max - limits.X.Min < 0.05)
                zoom = 16;
            else if (limits.X.Max - limits.X.Min < 0.1)
                zoom = 15;
            else if (limits.X.Max - limits.X.Min < 0.2)
                zoom = 14;
            else if (limits.X.Max - limits.X.Min < 0.5)
                zoom = 13;
            else if (limits.X.Max - limits.X.Min < 1)
                zoom = 12;
            else if (limits.X.Max - limits.X.Min < 2)
                zoom = 11;
            else if (limits.X.Max - limits.X.Min < 5)
                zoom = 10;
            else if (limits.X.Max - limits.X.Min < 10)
                zoom = 9;
            else if (limits.X.Max - limits.X.Min < 20)
                zoom = 8;
            else if (limits.X.Max - limits.X.Min < 50)
                zoom = 7;
            else if (limits.X.Max - limits.X.Min < 100)
                zoom = 6;
            else if (limits.X.Max - limits.X.Min < 200)
                zoom = 5;
            else
                zoom = 4;

            static int lastZoom = zoom;
            if (zoom != lastZoom) {
                fetcher.ClearJobs();
                for (auto& entry : tileCache)
                    entry.second.SetFetching(false);
                lastZoom = zoom;
            }

            int minX = MercatorProjection::ConvertEPSG3857ToTileX(limits.X.Min, zoom);
            int maxX = MercatorProjection::ConvertEPSG3857ToTileX(limits.X.Max, zoom);
            int minY = MercatorProjection::ConvertEPSG3857ToTileY(limits.Y.Max, zoom);
            int maxY = MercatorProjection::ConvertEPSG3857ToTileY(limits.Y.Min, zoom);

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
                        fetcher.Fetch({{zoom, tileX, tileY}}, [&texture](const OsmTileCoord& c, const std::vector<std::byte>& blob) {
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
                spec.MarkerSize = 5.0f;
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