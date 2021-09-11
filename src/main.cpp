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

void ProcessRoute(const char* path)
{
    Route route;
    Json::Value routeConfig;
    Load(path, routeConfig);    
    
    if(!routeConfig["takeoff"].isNull())
        Grib::SetDatabaseTime(DateTime::Parse(routeConfig["takeoff"].asCString()));

    if(!routeConfig["minutesUntilDeparture"].isNull())
    {
        auto currentTime = Grib::GetDatabaseTime();
        currentTime.AddMinutes(routeConfig["minutesUntilDeparture"].asDouble());
        Grib::SetDatabaseTime(currentTime);
    }

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

int main(int argc, const char* argv[])
{
    Grib::LoadRegistry();
    LoadAirports();
    cout << "[";
    for(auto i = 1; i < argc; i++)
    {
        ProcessRoute(argv[i]);
        if(i != argc - 1)
            cout << ",";
    }
    cout << "]" << endl;
    return 0;
}