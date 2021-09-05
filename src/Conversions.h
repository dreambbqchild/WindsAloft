#pragma once
#include <math.h>
#include <string>

double inline WindDirection(double u, double v) { return (180 / M_PI) * atan2(u, v) + 180; }

//m/s to KTS
double inline WindSpeed(double u, double v) { return sqrt(u * u + v * v) * 1.94384; }
double inline KelvinToCelcius(double tmp) { return tmp - 273.15; }
double inline InHgToPascals(double pressure) { return pressure * 3386.3886666667; }
double inline PascalsToInHg(double pressure) { return pressure / 3386.3886666667; }
double inline PascalsToMillibars(double pressure) { return pressure * 0.01; }
double inline MillibarsToInHg(double pressure) { return pressure / 33.863886666667; }
double inline MetersToFeet(double m) { return m * 3.281; }
double inline MillibarsToAltitude(double mb) { return (1 - pow((mb / 1013.25), 0.190284)) * 145366.45; }
double inline MetersToNauticalMiles(double m) {return m * 0.000539957;}
double inline NauticalMilesToMeters(double nm) { return nm * 1852; }

double MillibarLabelToAltitude(std::string label, double seaLevelPressure);
std::string MillibarLabelToAltitudeLabel(std::string label, double seaLevelPressure);
double MetersLabelToAltitude(std::string label);
std::string MetersLabelToAltitudeLabel(std::string label);
double LabelToAltitude(std::string label, double seaLevelPressure);