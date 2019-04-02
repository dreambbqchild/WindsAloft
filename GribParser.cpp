#include "GribParser.h"
#include "Conversions.h"
#include <sstream>

using namespace std;

unordered_set<string> dataKeys(
{ 
	U_COMPONENT_OF_WIND,
	V_COMPONENT_OF_WIND,
	TEMPERATURE,
	MSL_PRESSURE
});

GribParser::GribParser()
{
}

unordered_map<string, double> GribParser::Eval(int forecastIndex, double lat, double lon)
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

	lastEvalTime = "";
	keyMaster.clear();

	for (string line; getline(buffer, line); )
		ForecastDataFromLine(forecastIndex, line, forecastData);

	return forecastData;
}

void GribParser::ForecastDataFromLine(int forecastIndex, string line, unordered_map<string, double>& values)
{
	stringstream buffer;
	buffer << line;
	string token;
	string keyPrefix;
	string keySuffix;
	double value = 0;

	while (std::getline(buffer, token, ':'))
	{
		if (!lastEvalTime.length() && token.find("start_ft") == 0)
			lastEvalTime = token.substr(9);
		else if (dataKeys.find(token) != dataKeys.end())
			keyPrefix = token;
		else if (keyPrefix.length() && !keySuffix.length())
			keyMaster.emplace(keySuffix = token);
		else if (auto equals = token.find_last_of("="))
			value = atof(token.substr(equals + 1).c_str());
	}

	string key = keyPrefix + ":" + keySuffix;
	values[key] = value;
}


GribParser::~GribParser()
{
}
