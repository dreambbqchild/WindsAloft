#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include "DateTime.h"
#include "Route.h"

using namespace std;

Json::Value airports;

void LoadAirports() { Load("../airports.json", airports); }

Json::Value LoadAirport(string identifier)
{
    Json::Value result(airports[identifier]);
    result["identifier"] = identifier;
    return result;
}

void ProcessRoute()
{
    Route route;
    Json::Value routeConfig;
    Load("../route.json", routeConfig);    
    
    Grib::SetDatabaseTime(DateTime::Parse(routeConfig["takeoff"].asCString()));

    route.SetFrom(routeConfig["from"].asString());
    route.SetTo(routeConfig["to"].asString());

    for(const Json::Value& checkpoint : routeConfig["checkpoints"])
    {
        if(!checkpoint["altitude"].isNull())
            route.SetAltitude(checkpoint["altitude"].asDouble());

        if(!checkpoint["cas"].isNull())
            route.SetCalibratedAirspeed(checkpoint["cas"].asDouble());

        route.AddCheckpoint(checkpoint["nm"].asDouble());
    }

    route.AddFinalCheckpoint();
    
    cout << route;
}

int main()
{
    Grib::LoadRegistry();
    LoadAirports();
    ProcessRoute();
    return 0;
}