#pragma once
#include <string>
#include <fstream>

std::string ReadFile(const char* path, std::ios::openmode = std::ios::binary);

std::string ReadFileExtension(const char* path);