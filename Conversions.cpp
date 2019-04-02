#include "Conversions.h"
#include <sstream>

using namespace std;

double MillibarLabelToAltitude(string label, double seaLevelPressure)
{
	auto value = atof(label.substr(0, label.length() - 3).c_str());
	value = MillibarsToInHg(value);

	return (PascalsToInHg(seaLevelPressure) - value) * 1000;
}

string MillibarLabelToAltitudeLabel(string label, double seaLevelPressure)
{
	stringstream stream;
	stream << (int)MillibarLabelToAltitude(label, seaLevelPressure);
	return stream.str();
}

double MetersLabelToAltitude(string label)
{
	auto value = atof(label.substr(0, label.length() - 23).c_str());
	return MetersToFeet(value);
}

string MetersLabelToAltitudeLabel(string label)
{
	stringstream stream;
	stream << (int)MetersLabelToAltitude(label);
	return stream.str();
}

double LabelToAltitude(string label, double seaLevelPressure)
{
	if (label.find(" mb") == label.length() - 3)
		return MillibarLabelToAltitude(label, seaLevelPressure);
	else if (label.find(" m above mean sea level") == label.length() - 23)
		return MetersLabelToAltitude(label);

	return 0;
}
