#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string>
#include <vector>

namespace Filesystem
{
  std::vector<char> readFile(const std::string &filename);
}

#endif // FILESYSTEM_HPP
