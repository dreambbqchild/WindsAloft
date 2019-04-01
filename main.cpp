#include <iostream>
#include <iomanip>
#include <exception>
#include <cmath>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <experimental/filesystem>
#include <stdlib.h>
#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/GeodesicLine.hpp>
#include <GeographicLib/Constants.hpp>
#include <json/json.h>
#include <curl/curl.h>

using namespace std;
using namespace GeographicLib;
namespace fs = std::experimental::filesystem;

Json::Value airports;

#define FORECAST_HOURS 14

const double segmentLength = 1852 * 10;//10 intervals after m to nm conversion.

Geodesic geod(Constants::WGS84_a(), Constants::WGS84_f());

Json::Value checkpointForecast[FORECAST_HOURS]; 

unordered_set<string> keyMaster;

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
	while(getline(queryString, token, '&')) 
	{
		stringstream kvp;
		kvp << token;
		
		string key;
		string innerToken;
		
		while(getline(kvp, innerToken, '=')) 
		{
			if(key.length())
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
	ifs.open (getenv("AIRPORT_DB_PATH"), ifstream::in);
	ifs >> airports;
}

void ForecastDataFromLine(int forecastIndex, string line, unordered_map<string, double>& values) 
{	
	stringstream buffer;
	buffer << line;
	string token;
	string keyPrefix;
	string keySuffix;
	double value = 0;
	
	while(std::getline(buffer, token, ':')) 
	{
		if(!checkpointForecast[forecastIndex] && token.find("start_ft") == 0) {
			Json::Value checkpoints(Json::arrayValue); 
			checkpointForecast[forecastIndex]["time"] = token.substr(9);
			checkpointForecast[forecastIndex]["checkpts"] = checkpoints;
		}
		else if(token == "UGRD" || token == "VGRD" || token == "TMP" || token == "MSLET")
			keyPrefix = token;
		else if(keyPrefix.length() && !keySuffix.length())
			keyMaster.emplace(keySuffix = token);
		else if(auto equals = token.find_last_of("="))
			value = atof(token.substr(equals + 1).c_str());
	}
	
	string key = keyPrefix + ":" + keySuffix;
	values[key] = value;
}

double WindDirection(double u, double v)
{
	return (180/3.14) * atan2(u, v) + 180;	
}

double WindSpeed(double u, double v)
{
	                              //m/s to KTS
	return sqrt(u * u + v * v) * 1.94384;
}

double KelvinToCelcius(double tmp)
{
	return tmp - 273.15;
}

double PascalsToInHg(double pressure)
{
	return pressure / 3386.389;
}

double MillibarsToInHg(double pressure)
{
	return pressure / 33.86389;
}

double MetersToFeet(double m)
{
	return m = 3.281;
}

string MillibarLabelToAltitudeLabel(string label, double seaLevelPressure)
{
	stringstream stream;
	auto value = atof(label.substr(0, label.length() - 3).c_str());
	value = MillibarsToInHg(value);
	
	stream << (int)((seaLevelPressure - value) * 1000);
	return stream.str();
}

string MetersLabelToAltitudeLabel(string label)
{
	stringstream stream;
	auto value = atof(label.substr(0, label.length() - 23).c_str());
	value = round(MetersToFeet(value));
	
	stream << (int)value;
	return stream.str();
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

unordered_map<string, double> ParseGrib(int forecastIndex, double lat, double lon)
{
	int c = 0;
	stringstream stream, buffer, hour;
	unordered_map<string, double> forecastData;
	
	hour << "hour" << forecastIndex;
	
	stream << getenv("WGRIB2_LOCATION") << " " << getenv("WGRIB2_DATA_LOCATION") << "/" << (forecastIndex == 0 ? "analysis" : hour.str()) << ".grb -start_ft -s -lon " << lon << " " << lat;
	
	auto pipe = popen(stream.str().c_str(), "r");
	
	while ((c = fgetc(pipe)) != EOF)
		buffer << (char)c;

	pclose(pipe);	
	
	for (string line; getline(buffer, line); )
        ForecastDataFromLine(forecastIndex, line, forecastData);

	return forecastData;
}

Json::Value CheckpointData(unordered_map<string, double>& values, string key, double trueAirspeed, double* trueCourse)
{	
	Json::Value data;
	auto u = values["UGRD:" + key];
	auto v = values["VGRD:" + key];
	auto t = values["TMP:" + key];
	auto windDirection = WindDirection(u, v);
	auto iWindDirection = (int)round(windDirection);
	auto windSpeed = WindSpeed(u, v);
	
	data["windDir"] = iWindDirection == 0 ? 360 : iWindDirection;
	data["windSpd"] = (int)round(windSpeed);
	data["temp"] = (int)round(KelvinToCelcius(t));
	
	if(trueCourse)
	{
		double windCorrectionAngle = WindCorrectionAngle(windDirection, windSpeed, trueAirspeed, *trueCourse);
		data["WCA"] = (int)round(windCorrectionAngle);
		data["groundSpd"] = (int)round(GroundSpeed(windCorrectionAngle, windSpeed, trueAirspeed));
	}
	
	return data;
}

Json::Value AddCheckpointValue(int forecastIndex, int checkpointIndex, double trueAirspeed, double lat, double lon, double* trueCourse = nullptr)
{
	auto values = ParseGrib(forecastIndex, lat, lon);
	auto seaLevelPressure = PascalsToInHg(values["MSLET:mean sea level"]);
	Json::Value obj;
	Json::Value altitudes;

	obj["inHg"] = seaLevelPressure;

	for(auto itr = keyMaster.begin(); itr != keyMaster.end(); itr++)
	{
		if(*itr == "mean sea level")
			continue;
		
		auto data = CheckpointData(values, *itr, trueAirspeed, trueCourse);
		if((*itr).find(" mb") == (*itr).length() - 3)
			altitudes[MillibarLabelToAltitudeLabel(*itr, seaLevelPressure)] = data;
		else if((*itr).find(" m above mean sea level") == (*itr).length() - 23)
			altitudes[MetersLabelToAltitudeLabel(*itr)] = data;
		else
			obj[*itr] = data; 
	}		
	
	obj["altitudes"] = altitudes;
	checkpointForecast[forecastIndex]["checkpts"][checkpointIndex] = obj;	
	
	return obj;
}

void AddCheckpointValue(int forecastIndex, int checkpointIndex, double trueAirspeed, const GeodesicLine& line)
{
	double lat, lon;
	line.Position(checkpointIndex * segmentLength, lat, lon);
			
	double trueCourse = TrueCourse(lat, lon, line, checkpointIndex);
	AddCheckpointValue(forecastIndex, checkpointIndex, trueAirspeed, lat, lon, &trueCourse);
}

int GetMagneticVariation(double lat, double lon)
{
	int c = 0;
	stringstream stream, buffer;
	stream << "curl \"https://www.ngdc.noaa.gov/geomag-web/calculators/calculateDeclination?lat1=" << lat << "&lon1=" << lon << "&resultFormat=csv\" 2>/dev/null";
	auto pipe = popen(stream.str().c_str(), "r");
	
	while ((c = fgetc(pipe)) != EOF)
		buffer << (char)c;

	pclose(pipe);	
	
	for (string line; getline(buffer, line); )
	{
		if(line.length() > 0 && line[0] == '#')
			continue;
		
		stringstream csv;
		csv << line;
		string token;
		auto index = 0;
		while(getline(csv, token, ',')) 
		{
			if(index++ < 4)
				continue;
	
			return (int)round(atof(token.c_str()) * -1);
		}
	}
	
	return -360;
}

bool PrintJSON(string fileName)
{
	if(fs::exists(fileName))
	{
		string line;	
		ifstream ifs (fileName, ifstream::in);

		while(getline(ifs, line)) 
			cout << line;
		
		ifs.close();
		return true;
	}
	
	return false;
}

int main(int argc, char* argv[]) 
{
	auto queryString = ParseQueryString();
	
	if(!queryString["trueAirspeed.cruise"].length() || !queryString["to"].length() || !queryString["from"].length())
		return 0;
	
	double trueAirspeedAtCruise = atof(queryString["trueAirspeed.cruise"].c_str());
	stringstream fileName;
	fileName << getenv("FLIGHT_PLAN_CACHE") << "/" << queryString["from"] << "to" << queryString["to"] << "at" << queryString["trueAirspeed.cruise"] << ".json";
	
	if(PrintJSON(fileName.str()))
		return 0;
	
	try {
		LoadAirports();
		Json::Value result;

		result["from"] = queryString["from"];
		result["to"] = queryString["to"];

		double
			lat1 = airports[queryString["from"]]["latitude"].asDouble(), lon1 = airports[queryString["from"]]["longitude"].asDouble(),
			lat2 = airports[queryString["to"]]["latitude"].asDouble(), lon2 = airports[queryString["to"]]["longitude"].asDouble();

		const auto line = geod.InverseLine(lat1, lon1, lat2, lon2);

		for(auto forecastIndex = 0; forecastIndex < FORECAST_HOURS; forecastIndex++)
			AddCheckpointValue(forecastIndex, 0, trueAirspeedAtCruise, line);

		result["nmDistance"] = (line.Distance() * 0.000539957);

		int num = int(ceil(line.Distance() / segmentLength));
		for (int checkpointIndex = 1; checkpointIndex < num; checkpointIndex++) 
		{
			double lat, lon;
			line.Position(checkpointIndex * segmentLength, lat, lon);

			for(auto forecastIndex = 0; forecastIndex < FORECAST_HOURS; forecastIndex++)
				AddCheckpointValue(forecastIndex, checkpointIndex, trueAirspeedAtCruise, line);
		}

		Json::Value metadata(Json::arrayValue); 
		for(auto index = 0; index <= num; index++)
		{
			Json::Value obj;
			double lat, lon;
			line.Position(index * segmentLength, lat, lon);

			obj["trueCourse"] = (int)round(TrueCourse(lat, lon, line, index));
			obj["magVar"] = GetMagneticVariation(lat, lon);
			obj["lat"] = lat;
			obj["long"] = lon;

			metadata[index] = obj;
		}

		result["checkpointMetadata"] = metadata;

		for(auto i = 0; i < FORECAST_HOURS; i++) 
		{
			AddCheckpointValue(i, num, trueAirspeedAtCruise, lat2, lon2);
			result["forecasts"][i] = checkpointForecast[i];
			result["forecasts"][i]["time"] = checkpointForecast[i]["time"].asString();
		}

		Json::StreamWriterBuilder builder;
		builder["indentation"] = "";
		unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

		ofstream ofs (fileName.str(), ofstream::out);
		writer->write(result, &ofs);
		ofs.close();

		PrintJSON(fileName.str());
	} catch (const exception& e) {
		cerr << "Caught exception: " << e.what() << "\n";
		return 1;
	}
}
