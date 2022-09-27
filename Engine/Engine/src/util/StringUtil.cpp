#include "StringUtil.h"

std::vector<std::string> TokenizeString(std::string str, std::string delim)
{
	std::vector<std::string> tokens;

	size_t prevPos = 0;
	size_t pos = str.find(delim);

	size_t delimLength = delim.length();
	
	while(pos != std::string::npos)
	{
		tokens.push_back(str.substr(prevPos, pos - prevPos));
		prevPos = pos + delimLength;
		pos = str.find(delim, prevPos);
	}

	tokens.push_back(str.substr(prevPos, str.length() - prevPos));

	return tokens;
}

unsigned int GetOccurences(std::string str, const char* characters)
{
	unsigned int count = 0;
	size_t pos = str.find(characters);

	while(pos != std::string::npos)
	{
		pos = str.find(characters, pos);
		count++;
	}

	return count;

}
