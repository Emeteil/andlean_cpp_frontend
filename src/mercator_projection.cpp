#include "mercator_projection.h"

double MercatorProjection::ToEPSG3857_X(double lon_epsg4326)
{
    return lon_epsg4326;
}

double MercatorProjection::ToEPSG3857_Y(double lat_epsg4326)
{
    return std::log(std::tan(lat_epsg4326) + 1 / std::cos(lat_epsg4326));
}

double MercatorProjection::NormalizeEPSG3857_X(double x_epsg3857)
{
    return 0.5 + x_epsg3857 / 360;
}

double MercatorProjection::NormalizeEPSG3857_Y(double y_epsg3857)
{
    return 0.5 - y_epsg3857 / 2 * M_PI;
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

double MercatorProjection::ConvertEPSG4326ToEPSG3857_X(double lon_epsg4326, int zoom)
{

}

double MercatorProjection::ConvertEPSG4326ToEPSG3857_Y(double lat_epsg4326, int zoom)
{

}