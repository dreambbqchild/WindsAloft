#include "Grib.h"
#include "DataLoader.h"

#include <iostream>

using namespace std;

Json::Value registry;

const tm zero = {};
DateTime Grib::databaseAtTime(zero);

Json::Value::Members members;
Json::Value::Members::iterator Grib::itr;

void Grib::ConfigureDatabaseForTime()
{
    databaseAtTime = DateTime::Parse(registry[(*itr)].asString().c_str());
    grib_init((*itr).c_str());
}

void Grib::LoadRegistry() 
{
    Load("../registry.json", registry); 
    members = registry.getMemberNames();
    itr = members.begin();
    ConfigureDatabaseForTime();
}

void Grib::SetDatabaseTime(const DateTime& dateTime) 
{ 
    while(databaseAtTime.DiffInMinutes(dateTime) < -60)
    { 
        itr++;
        if(itr == members.end())
        {
            cerr << "No forecast available for " << dateTime << endl;
            exit(0);
        }
        ConfigureDatabaseForTime();
    }

    databaseAtTime = dateTime;
}