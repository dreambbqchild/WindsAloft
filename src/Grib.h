#pragma once

extern "C"
{
#include "grib_repo.h"
}

#include <json/json.h>

#include "DateTime.h"

#define FORECAST_HOURS 14
#define U_COMPONENT_OF_WIND "UGRD"
#define V_COMPONENT_OF_WIND "VGRD" 
#define TEMPERATURE "TMP"
#define MSL_PRESSURE "MSLET"
#define MEAN_SEA_LEVEL "mean sea level"

extern Json::Value registry;

class Grib
{
    private:
        static DateTime databaseAtTime;
        static Json::Value::Members::iterator itr;
        static void ConfigureDatabaseForTime();

    public:
        static void LoadRegistry();
        static void SetDatabaseTime(const DateTime& dateTime);
        static DateTime GetDatabaseTime(){ return databaseAtTime; };
        static inline double GetValue(const char* variableName, const char* location, double lat, double lon) { return grib_value(variableName, location, lat, lon); }
};