#pragma once
#include <string>
#include <optional>

std::optional<std::string> ReadFile(const char* path);

std::optional<std::string> ReadFileExtension(const char* path);