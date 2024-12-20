#ifndef API_COMMON_H
#define API_COMMON_H

#include <cstdint>

#pragma pack(push, 1)

struct GeoPosition
{
    double latitude;
    double longitude;
    double altitude;
    GeoPosition()
        : latitude(0.0)
        , longitude(0.0)
        , altitude(0.0)
    { };

    GeoPosition(const double lat, const double lon, const double alt)
        : latitude(lat)
        , longitude(lon)
        , altitude(alt)
    { };
};

#pragma pack(pop)

#endif // API_COMMON_H
