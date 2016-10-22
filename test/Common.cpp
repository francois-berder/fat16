#include <cstdlib>
#include "Common.hpp"


void restore_image(const std::string &path)
{
    std::string cmd = "git checkout ";
    cmd += path;
    system(cmd.c_str());
}
