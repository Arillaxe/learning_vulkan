#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <fstream>
#include <vector>
#include <iostream>

namespace Filesystem
{
  std::vector<char> readFile(const std::string &filename)
  {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
      throw std::runtime_error("Failed to open file: " + filename);
    }

    std::streamsize fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), fileSize);

    if (!file)
    {
      throw std::runtime_error("Failed to read file: " + filename);
    }

    return buffer;
  }
}

#endif // FILESYSTEM_HPP
