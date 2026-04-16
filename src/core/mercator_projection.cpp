#include "core/mercator_projection.h"

double MercatorProjection::DegreesToRadians(double degrees)
{
    return degrees * M_PI / 180;
}

double MercatorProjection::ToEPSG3857_X(double lon_epsg4326)
{
    return lon_epsg4326;
}

double MercatorProjection::ToEPSG3857_Y(double lat_epsg4326)
{
    return std::log(std::tan(DegreesToRadians(lat_epsg4326)) + 1 / std::cos(DegreesToRadians(lat_epsg4326))) * (180.0 / M_PI);
}

double MercatorProjection::NormalizeEPSG3857_X(double x_epsg3857)
{
    return 0.5 + x_epsg3857 / 360;
}

double MercatorProjection::NormalizeEPSG3857_Y(double y_epsg3857)
{
    return 0.5 - y_epsg3857 / 360;
}

int MercatorProjection::ZoomToTiles(int zoom)
{
    return 1 << zoom;
}

double MercatorProjection::GetCoordinateX(double x_epsg3857_normalized, int tiles)
{
    return tiles * x_epsg3857_normalized;
}

double MercatorProjection::GetCoordinateY(double y_epsg3857_normalized, int tiles)
{
    return tiles * y_epsg3857_normalized;
}

int MercatorProjection::ConvertEPSG4326ToEPSG3857_X(double lon_epsg4326, int zoom)
{
    return static_cast<int>(std::floor(GetCoordinateX(NormalizeEPSG3857_X(ToEPSG3857_X(lon_epsg4326)), ZoomToTiles(zoom))));
}

int MercatorProjection::ConvertEPSG4326ToEPSG3857_Y(double lat_epsg4326, int zoom)
{
    return static_cast<int>(std::floor(GetCoordinateY(NormalizeEPSG3857_Y(ToEPSG3857_Y(lat_epsg4326)), ZoomToTiles(zoom))));
}

int MercatorProjection::ConvertEPSG3857ToTileX(double x_epsg3857, int zoom)
{
    return static_cast<int>(std::floor(GetCoordinateX(NormalizeEPSG3857_X(x_epsg3857), ZoomToTiles(zoom))));
}

int MercatorProjection::ConvertEPSG3857ToTileY(double y_epsg3857, int zoom)
{
    return static_cast<int>(std::floor(GetCoordinateY(NormalizeEPSG3857_Y(y_epsg3857), ZoomToTiles(zoom))));
}

double MercatorProjection::TileXToEPSG3857(int tileX, int zoom)
{
    return (tileX / static_cast<double>(ZoomToTiles(zoom)) - 0.5) * 360.0;
}

double MercatorProjection::TileYToEPSG3857(int tileY, int zoom)
{
    return (0.5 - tileY / static_cast<double>(ZoomToTiles(zoom))) * 360.0;
}