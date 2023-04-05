#include <filesystem>
#include <fstream>
#include <string>

typedef long long int FilePosition;

struct DiskPtr
{
    std::string filename;
    FilePosition posinfile;
};