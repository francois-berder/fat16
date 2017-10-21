/*
 * Copyright (C) 2017  Francois Berder <fberder@outlook.fr>
 *
 * This file is part of fat16.
 *
 * fat16 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fat16 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fat16.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <cstdlib>
#include <stdexcept>
#include "Common.hpp"
#include "linux_hal.h"


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
