#pragma once

#include "Airports.h"
#include "Conversions.h"
#include "FlightPlanCalcs.h"
#include "Grib.h"

class Route
{
    private:
        double altitude, latFrom, latTo, lonFrom, lonTo, cas, nmTraveled;
        std::string from;
        std::string to;
        Json::Value result;
        Json::Value checkpoints;
        GeographicLib::GeodesicLine line;
        
        void InitRoute();
        Json::Value AddCheckpoint(const char* level, double seaLevelPressure, double lat, double lon);

    public:
        Route() : altitude(0), latFrom(0), latTo(0), lonFrom(0), lonTo(0), cas(0), nmTraveled(0), checkpoints(Json::arrayValue) {}
        inline void SetFrom(std::string from) { this->from = from; InitRoute();}
        inline void SetTo(std::string to) { this->to = to; InitRoute();}
        inline void SetAltitude(double altitude) { this->altitude = altitude; }
        inline void SetCalibratedAirspeed(double cas) { this->cas = cas; }

        void AddCheckpoint(double nm, std::string name);
        void AddFinalCheckpoint();

        friend std::ostream& operator <<(std::ostream& os, Route& route);
};