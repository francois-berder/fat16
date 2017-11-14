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


#include "Common.hpp"
#include "MkdirTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"


MkdirTest::MkdirTest():
Test("MkdirTest")
{

}

void MkdirTest::init()
{
    restore_image();
    mount_image();
    system("mkdir /mnt/MUSIC");
    unmount_image();
    load_image();
}

bool MkdirTest::run()
{
    if (fat16_init(linux_dev, 0) < 0)
        return false;

    if (fat16_mkdir("/DATA") < 0)
        return false;
    if (fat16_open("/DATA", 'r') == 0)
        return false;
    if (fat16_mkdir("/TMP") < 0)
        return false;
    if (fat16_open("/TMP", 'w') == 0)
        return false;

    if (fat16_mkdir("/IMAGES") < 0)
        return false;
    if (fat16_mkdir("/IMAGES/JPEG") < 0)
        return false;
    if (fat16_mkdir("/IMAGES/PNG") < 0)
        return false;
    if (fat16_mkdir("/IMAGES/BMP") < 0)
        return false;

    if (fat16_mkdir("/MUSIC/MP3") < 0)
        return false;
    if (fat16_mkdir("/MUSIC/MP3/OLD") < 0)
        return false;
    if (fat16_mkdir("/MUSIC/MP3/NEW") < 0)
        return false;
    if (fat16_mkdir("/MUSIC/OGG") < 0)
        return false;

    return true;
}
