#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <fstream>
#include <iostream>

bool readFile(const std::string &filename, std::vector<char> &buffer)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    std::cerr << "failed to open file: " + filename << std::endl;

    return false;
  }

  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

  file.close();

  return true;
}

#endif // UTILS_HPP
