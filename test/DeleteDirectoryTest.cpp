#include <sys/stat.h>
#include "Common.hpp"
#include "DeleteDirectoryTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"


DeleteDirectoryTest::DeleteDirectoryTest():
Test("DeleteDirectoryTest")
{

}

void DeleteDirectoryTest::init()
{
    restore_image();
    mount_image();
    system("mkdir /mnt/DOCS");
    system("mkdir /mnt/DOCS/MUSIC");
    unmount_image();
    load_image();
}

bool DeleteDirectoryTest::run()
{
    if (fat16_init(linux_dev) < 0)
        return false;


    if (fat16_rmdir("/DOCS/MUSIC") < 0)
        return false;

    if (fat16_rmdir("/DOCS") < 0)
        return false;

    if (directory_exists("/DOCS/MUSIC"))
        return false;

    if (directory_exists("/DOCS"))
        return false;

    return true;
}

bool DeleteDirectoryTest::directory_exists(const std::string &dirpath)
{
    bool exists = false;
    mount_image();

    std::string path = "/mnt/" + dirpath;
    struct stat info;
    exists = stat(path.c_str(), &info) == 0;
    unmount_image();

    return exists;
}
