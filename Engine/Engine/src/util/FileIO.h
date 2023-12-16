#pragma once
#include <string>
#include <fstream>

std::string ReadFile(const std::string path, std::ios::openmode = std::ios::binary);

std::string ReadFileExtension(const std::string fileName);