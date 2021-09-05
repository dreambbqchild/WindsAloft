#pragma once
#include <fstream>
#include <string>
#include <json/json.h>

inline void Load(std::string path, Json::Value& target)
{
	std::ifstream ifs;
	ifs.open(path, std::ifstream::in);
	ifs >> target;    
}