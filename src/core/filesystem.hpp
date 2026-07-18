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
      throw std::runtime_error("failed to open file: " + filename);
    }

    std::vector<char> buffer;

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

    file.close();

    return buffer;
  }
}

#endif // FILESYSTEM_HPP
