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
#include "RmdirTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"

RmdirTest::RmdirTest():
Test("RmdirTest")
{
}

void RmdirTest::init()
{
    restore_image();
    mount_image();
    system("mkdir -p /mnt/TMP");
    system("mkdir -p /mnt/DATA/PNG");
    unmount_image();
    load_image();
}

bool RmdirTest::run()
{
    if (fat16_init(linux_dev, 0) < 0)
        return false;

    if (fat16_rmdir("/TEST") == 0)
        return false;

    if (fat16_rmdir("/TMP") != 0)
        return false;

    if (fat16_rmdir("/DATA/") == 0)
        return false;

    if (fat16_rmdir("/DATA/BMP") == 0)
        return false;

    if (fat16_rmdir("/DATA/PNG") != 0)
        return false;

    if (fat16_rmdir("/DATA") != 0)
        return false;

    return true;
}
