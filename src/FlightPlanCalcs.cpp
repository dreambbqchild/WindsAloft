#include <cmath>
#include <GeographicLib/Constants.hpp>
#include <GeographicLib/MagneticModel.hpp>
#include "FlightPlanCalcs.h"
#include "Conversions.h"

using namespace std;
using namespace GeographicLib;

const double radians = M_PI / 180;
const double metersInNauticalMile = 1852;

Geodesic geod(Constants::WGS84_a(), Constants::WGS84_f());

inline double PressureLapseRate(double temperature, double pressure) { return 96 * (temperature + 273) / pressure; }	
inline double PressureAlt(double altitude, double pressure, double lapseRate) { return altitude + (1013 - pressure) * lapseRate; }
inline double TempISApa(double pressureAlt) { return 15 - (pressureAlt / 1000) * 2; }
inline double DensityAlt(double pressureAlt, double temperature, double a) { return pressureAlt + 120 * (temperature - a); }

double WindCorrectionAngle(double windDirection, double windSpeed, double trueAirspeed, double trueCourse)
{
	if(!trueAirspeed)
		return 0;
	
	auto radianCourse = radians * ((int)round(trueCourse) % 360);
	auto radianWindDir = radians * windDirection;
	auto c = windSpeed / trueAirspeed * sin(radianWindDir - radianCourse);
	auto l = radianCourse + asin(c);
	
	return 1 / radians * (-1 * atan2(windSpeed * sin(l - radianWindDir), trueAirspeed - windSpeed * cos(l - radianWindDir)));
}

double GroundSpeed(double windDirection, double windSpeed, double trueHeading, double trueAirspeed)
{
	windDirection = windDirection * Math::degree();
	trueHeading = trueHeading * Math::degree();

	return (trueAirspeed * sqrt(1 - pow(windSpeed / trueAirspeed, 2))) - (windSpeed * cos(windDirection - trueHeading));
}

double TrueCourse(const GeodesicLine& line, double lat1, double lon1, double lat2, double lon2)
{
	const auto lineFragment = geod.InverseLine(lat1, lon1, lat2, lon2);
	auto degrees = lineFragment.Azimuth();
	if(degrees < 0)
		degrees += 360;
	return degrees;
}

double TrueAirspeed(double indicatedAirspeed, double pressure, double temperature, double altitude)
{
    temperature = KelvinToCelcius(temperature);
    pressure *= 0.01;
    auto pressureAlt = PressureAlt(altitude, pressure, PressureLapseRate(temperature, pressure));
    auto densityAlt = DensityAlt(pressureAlt, temperature, TempISApa(pressureAlt));    
    return round(indicatedAirspeed * (1 / pow(pow((288.15 - .0019812 * densityAlt) / 288.15, 1 / .234969), .5)));
}

int GetMagneticVariation(int year, double lat, double lon)
{
	MagneticModel mag("wmm2020");
	double Bx, By, Bz;
	mag(year, lat, lon, 0, Bx, By, Bz);

	double H, F, D, I;
	MagneticModel::FieldComponents(Bx, By, Bz, H, F, D, I);
	return (int)(round(D) * -1);
}

double DistanceBetween(double lat1, double lon1, double lat2, double lon2)
{
    auto line = geod.InverseLine(lat1, lon1, lat2, lon2);
    return MetersToNauticalMiles(line.Distance());
}