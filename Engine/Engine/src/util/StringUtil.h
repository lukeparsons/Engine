#pragma once
#include <vector>
#include <string>

std::vector<std::string> TokenizeString(std::string str, std::string delim);

unsigned int GetOccurences(std::string str, std::string characters);