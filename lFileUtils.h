#ifndef L_FILE_UTILS_H
#define L_FILE_UTILS_H

#include <string>
#include <vector>

std::vector<char> lLoadBinaryFile(const std::string &filename);
std::string lLoadTextFile(const std::string &filename);

#endif // L_FILE_UTILS_H
