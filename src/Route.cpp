#include "Route.h"
#include "Grib.h"
using namespace std;

double Calc(Json::Value& high, Json::Value& low, string property, double percentage)
{
    return round((high[property].asDouble() - low[property].asDouble()) * percentage + low[property].asDouble());
}

void ManageLowAndHigh(double altitude, const Json::Value& data, Json::Value& low, Json::Value& high)
{
    if(low.isNull() && data["altitude"].asDouble() <= altitude)
        low = data;

    if(high.isNull() && data["altitude"].asDouble() >= altitude)
        high = data;

    if(low["altitude"].asDouble() <= altitude && data["altitude"].asDouble() <= altitude)
    {
        auto lowDiff = altitude - low["altitude"].asDouble();
        auto dataDiff = altitude - data["altitude"].asDouble();
        if(dataDiff < lowDiff)
            low = data;
    }

    if(high["altitude"].asDouble() >= altitude && data["altitude"].asDouble() >= altitude)
    {
        auto highDiff = high["altitude"].asDouble() - altitude;
        auto dataDiff = data["altitude"].asDouble() - altitude;
        if(dataDiff > highDiff)
            low = data;
    }
}

Json::Value MakeElapsedTime(double minutes)
{
    Json::Value elapsed;
    auto wholeMinute = 0.0;
    auto fractionalMinutes = modf(minutes, &wholeMinute);

    elapsed["minutes"] = wholeMinute;
    elapsed["seconds"] = round(fractionalMinutes * 60);
    return elapsed;
} 

void Route::InitRoute()
{
    if(!from.length() || !to.length())
        return;

    result["from"] = LoadAirport(from);
    result["to"] = LoadAirport(to);

    altitude = result["from"]["elevation"].asDouble();
    latFrom = result["from"]["lat"].asDouble();
    lonFrom = result["from"]["lon"].asDouble();
    latTo = result["to"]["lat"].asDouble();
    lonTo = result["to"]["lon"].asDouble();

    result["nm"] = round(DistanceBetween(latFrom, lonFrom, latTo, lonTo));

    line = geod.InverseLine(latFrom, lonFrom, latTo, lonTo);

    AddCheckpoint(numeric_limits<double>::epsilon(), "Depart " + from);
}

Json::Value Route::AddCheckpoint(const char* level, double seaLevelPressure, double lat, double lon)
{
	Json::Value data;

	auto u = Grib::GetValue(U_COMPONENT_OF_WIND, level, lat, lon);
	auto v = Grib::GetValue(V_COMPONENT_OF_WIND, level, lat, lon);
	auto t = Grib::GetValue(TEMPERATURE, level, lat, lon);
	auto windDirection = WindDirection(u, v);
	auto iWindDirection = (int)round(windDirection);
	auto windSpeed = WindSpeed(u, v);

    auto trueCourse = TrueCourse(line, latFrom, lonFrom, lat, lon);
	auto trueAirspeed = TrueAirspeed(cas, seaLevelPressure, t, altitude);
	auto windCorrectionAngle = WindCorrectionAngle(windDirection, windSpeed, trueAirspeed, trueCourse);

    data["altitude"] = (int)round(LabelToAltitude(level, seaLevelPressure));
    data["trueCourse"] = (int)round(trueCourse);
	data["windDir"] = iWindDirection == 0 ? 360 : iWindDirection;
	data["windSpd"] = (int)round(windSpeed);
	data["temp"] = (int)round(KelvinToCelcius(t));
	data["WCA"] = (int)round(windCorrectionAngle);
	data["groundSpd"] = (int)round(GroundSpeed(windDirection, windSpeed, trueCourse + windCorrectionAngle, trueAirspeed));
	data["trueAirspeed"] = (int)round(trueAirspeed);

	return data;
}

void Route::AddCheckpoint(double nm, string name)
{
    if(nm == 0)
        return;

    Json::Value low, high;
    Json::Value checkpointData;

    double lat = 0, lon = 0;
    line.Position(NauticalMilesToMeters(nmTraveled) + NauticalMilesToMeters(nm), lat, lon);

    auto currentTime = Grib::GetDatabaseTime();

    auto seaLevelPressure = Grib::GetValue(MSL_PRESSURE, MEAN_SEA_LEVEL, lat, lon);
    for(auto i = 0; i < TOTAL_LEVELS; i++)
    {
        auto checkpointData = AddCheckpoint(levels[i], seaLevelPressure, lat, lon);
        ManageLowAndHigh(altitude, checkpointData, low, high);
    }

    auto range = (high["altitude"].asDouble() - low["altitude"].asDouble());
	auto percentage = (altitude - low["altitude"].asDouble()) / range;

    //Constants given current position
    checkpointData["name"] = name;
    checkpointData["altitude"] = altitude;
    checkpointData["lat"] = lat;
    checkpointData["lon"] = lon;
    checkpointData["magVar"] = GetMagneticVariation(currentTime.Year(), lat, lon);
    checkpointData["inHg"] = PascalsToInHg(seaLevelPressure);
    checkpointData["nm"] = round(nm);
    checkpointData["trueCourse"] = low["trueCourse"];

    //Interpolations
    checkpointData["groundSpd"] = fmax(0, Calc(high, low, "groundSpd", percentage));
    checkpointData["temp"] = Calc(high, low, "temp", percentage);
    checkpointData["windDir"] = Calc(high, low, "windDir", percentage);
    checkpointData["windSpd"] = Calc(high, low, "windSpd", percentage);
    checkpointData["trueAirspeed"] = Calc(high, low, "trueAirspeed", percentage);
    checkpointData["WCA"] = (int)round(WindCorrectionAngle(checkpointData["windDir"].asDouble(),
        checkpointData["windSpd"].asDouble(), 
        checkpointData["trueAirspeed"].asDouble(), 
        checkpointData["trueCourse"].asDouble()));

    checkpointData["heading"] = checkpointData["trueCourse"].asDouble() + checkpointData["magVar"].asDouble() + checkpointData["WCA"].asDouble();

    if(checkpointData["groundSpd"].asDouble())
    {
        auto minutes = nm / ((checkpointData["groundSpd"].asDouble() / 60.0));
        checkpointData["elapsed"] = MakeElapsedTime(minutes);
        currentTime.AddMinutes(minutes);
        nmTraveled += nm;
    }
    else
        checkpointData["elapsed"] = MakeElapsedTime(0);

    checkpointData["time"] = currentTime.ToString();
    Grib::SetDatabaseTime(currentTime);

    checkpoints.append(checkpointData);
}

void Route::AddFinalCheckpoint()
{
    altitude = result["to"]["elevation"].asDouble();
    AddCheckpoint(round(MetersToNauticalMiles(line.Distance()) - nmTraveled), "Arrive " + to);
    result["checkpoints"] = checkpoints;
}

std::ostream& operator <<(std::ostream& os, Route& route)
{
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

    writer->write(route.result, &os);

    return os;
}
