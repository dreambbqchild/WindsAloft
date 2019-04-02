#include <iostream>
#include <iomanip>
#include <exception>
#include <cmath>
#include <sstream>
#include <fstream>
#include <experimental/filesystem>
#include <stdlib.h>
#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/GeodesicLine.hpp>
#include <GeographicLib/Constants.hpp>
#include <GeographicLib/MagneticModel.hpp>
#include <curl/curl.h>

#include "GribParser.h"
#include "Conversions.h"

using namespace std;
using namespace GeographicLib;
namespace fs = std::experimental::filesystem;

Json::Value airports;
Json::Value checkpointForecast(Json::arrayValue);
Json::Value metadataValues(Json::arrayValue);

const double segmentLength = 1852 * 10;//10 intervals after m to nm conversion.

Geodesic geod(Constants::WGS84_a(), Constants::WGS84_f());

GribParser gribParser;

string GetDecodedValue(CURL *curl, string value)
{
	int outLength = 0;
	char* result = curl_easy_unescape(curl, value.c_str(), 0, &outLength);

	value = result;

	curl_free(result);

	return value;
}

unordered_map<string, string> ParseQueryString()
{
	unordered_map<string, string> result;

	CURL *curl = curl_easy_init();
	stringstream queryString;
	queryString << getenv("QUERY_STRING");

	string token;
	while (getline(queryString, token, '&'))
	{
		stringstream kvp;
		kvp << token;

		string key;
		string innerToken;

		while (getline(kvp, innerToken, '='))
		{
			if (key.length())
				result[key] = GetDecodedValue(curl, innerToken);
			else
				key = innerToken;
		}
	}

	curl_easy_cleanup(curl);

	return result;
}

void LoadAirports()
{
	ifstream ifs;
	ifs.open(getenv("AIRPORT_DB_PATH"), ifstream::in);
	ifs >> airports;
}

double WindCorrectionAngle(double windDirection, double windSpeed, double trueAirspeed, double trueCourse)
{
	trueCourse += 180;

	auto awaHeadwind = windDirection - 180 - trueCourse;
	auto awaTailwind = trueCourse - windDirection;
	auto awa = abs(awaHeadwind) < abs(awaTailwind) ? awaHeadwind : awaTailwind;
	auto awaRadians = awa * Math::degree();

	return asin((windSpeed * sin(awaRadians)) / trueAirspeed) / Math::degree();
}

double GroundSpeed(double windCorrectionAngle, double windSpeed, double trueAirspeed)
{
	windCorrectionAngle = windCorrectionAngle * Math::degree();
	return cos(windCorrectionAngle) * trueAirspeed + windSpeed;
}

double TrueCourse(double lat1, double lon1, const GeodesicLine& line, int checkpointIndex)
{
	double lat2, lon2;

	line.Position((checkpointIndex * segmentLength) + 1, lat2, lon2);

	const auto lineFragment = geod.InverseLine(lat1, lon1, lat2, lon2);
	return lineFragment.Azimuth();
}

double TrueAirspeed(double indicatedAirspeed, double pressure, double temperature)
{
	return indicatedAirspeed * sqrt(1.2754 / (pressure / (287.058 * temperature)));
}

int GetMagneticVariation(double lat, double lon)
{
	MagneticModel mag("wmm2015");
	double Bx, By, Bz;
	mag(2019, lat, lon, 0, Bx, By, Bz);

	double H, F, D, I;
	MagneticModel::FieldComponents(Bx, By, Bz, H, F, D, I);
	return (int)round(D);
}

Json::Value CheckpointData(unordered_map<string, double>& values, string key, double seaLevelPressure, double indicatedAirspeed, double trueCourse)
{
	Json::Value data;
	auto u = values[GRIB_KEY(U_COMPONENT_OF_WIND, key)];
	auto v = values[GRIB_KEY(V_COMPONENT_OF_WIND, key)];
	auto t = values[GRIB_KEY(TEMPERATURE, key)];
	auto windDirection = WindDirection(u, v);
	auto iWindDirection = (int)round(windDirection);
	auto windSpeed = WindSpeed(u, v);

	auto altitude = LabelToAltitude(key, seaLevelPressure);
	auto trueAirspeed = TrueAirspeed(indicatedAirspeed, seaLevelPressure - InHgToPascals(altitude / 1000.0), t);
	auto windCorrectionAngle = WindCorrectionAngle(windDirection, windSpeed, trueAirspeed, trueCourse);

	data["windDir"] = iWindDirection == 0 ? 360 : iWindDirection;
	data["windSpd"] = (int)round(windSpeed);
	data["temp"] = (int)round(KelvinToCelcius(t));
	data["WCA"] = (int)round(windCorrectionAngle);
	data["groundSpd"] = (int)round(GroundSpeed(windCorrectionAngle, windSpeed, trueAirspeed));
	data["trueAirspeed"] = (int)round(trueAirspeed);

	return data;
}

Json::Value AddCheckpointValue(int forecastIndex, int checkpointIndex, double indicatedAirspeed, const GeodesicLine& line, double lat, double lon)
{
	auto trueCourse = TrueCourse(lat, lon, line, checkpointIndex);
	auto values = gribParser.Eval(forecastIndex, lat, lon);
	auto seaLevelPressure = values[GRIB_KEY(MSL_PRESSURE, "mean sea level")];
	auto keyMaster = gribParser.GetKeys();
	Json::Value obj;
	Json::Value altitudes;

	obj["inHg"] = PascalsToInHg(seaLevelPressure);
	obj["time"] = gribParser.GetEvalTime();

	for (auto itr = keyMaster.begin(); itr != keyMaster.end(); itr++)
	{
		if (*itr == "mean sea level")
			continue;

		auto data = CheckpointData(values, *itr, seaLevelPressure, indicatedAirspeed, trueCourse);
		if ((*itr).find(" mb") == (*itr).length() - 3)
			altitudes[MillibarLabelToAltitudeLabel(*itr, seaLevelPressure)] = data;
		else if ((*itr).find(" m above mean sea level") == (*itr).length() - 23)
			altitudes[MetersLabelToAltitudeLabel(*itr)] = data;
		else
			obj[*itr] = data;
	}

	obj["altitudes"] = altitudes;
	checkpointForecast[forecastIndex]["checkpts"][checkpointIndex] = obj;

	if (forecastIndex == 0)
	{
		Json::Value metadata;
		metadata["trueCourse"] = (int)round(trueCourse);
		metadata["magVar"] = GetMagneticVariation(lat, lon);
		metadata["lat"] = lat;
		metadata["long"] = lon;

		metadataValues[checkpointIndex] = metadata;
	}

	return obj;
}

Json::Value AddCheckpointValue(int forecastIndex, int checkpointIndex, double indicatedAirspeed, const GeodesicLine& line)
{
	double lat, lon;
	line.Position(checkpointIndex * segmentLength, lat, lon);

	return AddCheckpointValue(forecastIndex, checkpointIndex, indicatedAirspeed, line, lat, lon);
}

bool PrintJSON(string fileName)
{
	if (fs::exists(fileName))
	{
		string line;
		ifstream ifs(fileName, ifstream::in);

		while (getline(ifs, line))
			cout << line;

		ifs.close();
		return true;
	}

	return false;
}

int main(int argc, char* argv[])
{
	auto queryString = ParseQueryString();

	if (!queryString["indicatedAirspeed"].length() || !queryString["to"].length() || !queryString["from"].length())
		return 0;

	double indicatedAirspeed = atof(queryString["indicatedAirspeed"].c_str());
	stringstream fileName;
	fileName << getenv("FLIGHT_PLAN_CACHE") << "/" << queryString["from"] << "to" << queryString["to"] << "at" << queryString["indicatedAirspeed"] << ".json";

	if (PrintJSON(fileName.str()))
		return 0;

	try {
		LoadAirports();
		Json::Value result;

		result["fromIdentifier"] = queryString["from"];
		result["toIdentifier"] = queryString["to"];
		result["fromName"] = airports[queryString["from"]]["name"];
		result["toName"] = airports[queryString["to"]]["name"];

		double
			lat1 = airports[queryString["from"]]["latitude"].asDouble(), lon1 = airports[queryString["from"]]["longitude"].asDouble(),
			lat2 = airports[queryString["to"]]["latitude"].asDouble(), lon2 = airports[queryString["to"]]["longitude"].asDouble();

		const auto line = geod.InverseLine(lat1, lon1, lat2, lon2);
		result["nmDistance"] = (line.Distance() * 0.000539957);
		int num = int(ceil(line.Distance() / segmentLength));

		for (auto forecastIndex = 0; forecastIndex < FORECAST_HOURS; forecastIndex++)
		{
			Json::Value checkpoints(Json::arrayValue);
			checkpointForecast[forecastIndex]["checkpts"] = checkpoints;
		}

		for (int checkpointIndex = 0; checkpointIndex < num; checkpointIndex++)
		{
			for (auto forecastIndex = 0; forecastIndex < FORECAST_HOURS; forecastIndex++)
				AddCheckpointValue(forecastIndex, checkpointIndex, indicatedAirspeed, line);
		}

		for (auto forecastIndex = 0; forecastIndex < FORECAST_HOURS; forecastIndex++)
			AddCheckpointValue(forecastIndex, num, indicatedAirspeed, line, lat2, lon2);

		result["forecasts"] = checkpointForecast;
		result["checkpointMetadata"] = metadataValues;

		Json::StreamWriterBuilder builder;
		builder["indentation"] = "";
		unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

		ofstream ofs(fileName.str(), ofstream::out);
		writer->write(result, &ofs);
		ofs.close();

		PrintJSON(fileName.str());
	}
	catch (const exception& e) {
		cerr << "Caught exception: " << e.what() << "\n";
		return 1;
	}
}
