#pragma once
#include <math.h>
#include <string>

double inline WindDirection(double u, double v)
{
	return (180 / M_PI) * atan2(u, v) + 180;
}

double inline WindSpeed(double u, double v)
{
	//m/s to KTS
	return sqrt(u * u + v * v) * 1.94384;
}

double inline KelvinToCelcius(double tmp)
{
	return tmp - 273.15;
}

double inline InHgToPascals(double pressure)
{
	return pressure * 3386.389;
}

double inline PascalsToInHg(double pressure)
{
	return pressure / 3386.389;
}

double inline MillibarsToInHg(double pressure)
{
	return pressure / 33.86389;
}

double inline MetersToFeet(double m)
{
	return m * 3.281;
}

double MillibarLabelToAltitude(std::string label, double seaLevelPressure);
std::string MillibarLabelToAltitudeLabel(std::string label, double seaLevelPressure);
double MetersLabelToAltitude(std::string label);
std::string MetersLabelToAltitudeLabel(std::string label);
double LabelToAltitude(std::string label, double seaLevelPressure);