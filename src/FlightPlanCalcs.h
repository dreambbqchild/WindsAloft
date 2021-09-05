#pragma once
#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/GeodesicLine.hpp>

extern GeographicLib::Geodesic geod;

double WindCorrectionAngle(double windDirection, double windSpeed, double trueAirspeed, double trueCourse);
double GroundSpeed(double windDirection, double windSpeed, double trueHeading, double trueAirspeed);
double TrueCourse(const GeographicLib::GeodesicLine& line, double lat1, double lon1, double lat2, double lon2);
double TrueAirspeed(double indicatedAirspeed, double pressure, double temperature);
int GetMagneticVariation(double lat, double lon);
double DistanceBetween(double lat1, double lon1, double lat2, double lon2);