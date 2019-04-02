#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <json/json.h>

#define FORECAST_HOURS 14
#define U_COMPONENT_OF_WIND "UGRD"
#define V_COMPONENT_OF_WIND "VGRD" 
#define TEMPERATURE "TMP"
#define MSL_PRESSURE "MSLET"

#define GRIB_KEY(variable, key) (variable + string(":") + key)

class GribParser
{
private:
	std::unordered_set<std::string> keyMaster;
	std::string lastEvalTime;

	void ForecastDataFromLine(int forecastIndex, std::string line, std::unordered_map<std::string, double>& values);

public:
	GribParser();

	std::unordered_map<std::string, double> Eval(int forecastIndex, double lat, double lon);
	std::string GetEvalTime() { return lastEvalTime; }
	std::unordered_set<std::string> GetKeys() { return keyMaster; }

	virtual ~GribParser();
};

