#include "StringUtil.h"
#include <sstream>

std::vector<std::string> TokenizeString(std::string str, const char delim)
{
	std::vector<std::string> tokens;

	if(str.empty() || str.find(delim) == std::string::npos)
	{
		return tokens;
	}

	std::istringstream issTokens(str);
	std::string token;
	while(std::getline(issTokens, token, delim))
	{
		tokens.push_back(token);
	}
	
	return tokens;
}
