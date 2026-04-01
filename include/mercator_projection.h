#include <cmath>

class MercatorProjection
{
    private:
        static double DegreesToRadians(double degrees);

        static double NormalizeEPSG3857_X(double x_epsg3857);
        static double NormalizeEPSG3857_Y(double y_epsg3857);

        static int ZoomToTiles(int zoom);

        static double GetCoordinateX(double x_epsg3857_normalized, int tiles);
        static double GetCoordinateY(double y_epsg3857_normalized, int tiles);
    
    public:
        static double ToEPSG3857_X(double lon_epsg4326);
        static double ToEPSG3857_Y(double lat_epsg4326);
        
        static int ConvertEPSG4326ToEPSG3857_X(double lon_epsg4326, int zoom);
        static int ConvertEPSG4326ToEPSG3857_Y(double lat_epsg4326, int zoom);

        static int ConvertEPSG3857ToTileX(double x_epsg3857, int zoom);
        static int ConvertEPSG3857ToTileY(double y_epsg3857, int zoom);
        static double TileXToEPSG3857(int tileX, int zoom);
        static double TileYToEPSG3857(int tileY, int zoom);
};