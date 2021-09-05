#pragma once

#define TOTAL_LEVELS 34

extern const char* levels[TOTAL_LEVELS];

void grib_init(const char* datafile);
double grib_value(const char* variableName, const char* location, double lat, double lon);