#pragma once
#include <string>
#include "../types/Either.h"
#include <fstream>

Either<std::string, std::string> ReadFile(const char* path);

Either<std::string, std::string> ReadFileExtension(const char* path);