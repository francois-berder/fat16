#include <cstdlib>
#include <stdexcept>
#include "Common.hpp"
#include "../driver/linux_hal.h"


static std::string image_path = "data/fs.img";

void mount_image()
{
    std::string cmd = "mount ";
    cmd += image_path;
    cmd += " /mnt";
    system(cmd.c_str());
}

void unmount_image()
{
    system("umount /mnt");
}

void restore_image()
{
    std::string cmd = "git checkout ";
    cmd += image_path;
    system(cmd.c_str());
}

void load_image()
{
    if (linux_load_image(image_path.c_str()) < 0)
        throw std::runtime_error("Failed to load image.");
}

void release_image()
{
    if (linux_release_image() < 0)
        throw std::runtime_error("Failed to load image.");
}
