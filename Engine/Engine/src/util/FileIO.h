#pragma once
#include <string>
#include "../types/Either.h"

Either<std::string, std::string> ReadFile(const char* path);

Either<std::string, std::string> ReadFileExtension(const char* path);