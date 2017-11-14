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


#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "Common.hpp"
#include "FilenameTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"


#define CHECK_NAME(NAME, EXP) do { \
    if (!check_name(NAME, EXP)) { \
        std::cerr << "Failed because of " << NAME << std::endl; \
        return false; \
    } \
 } while (0)

FilenameTest::FilenameTest():
Test("FilenameTest")
{

}

bool FilenameTest::run()
{
    if (fat16_init(linux_dev, 0) < 0)
        return false;

    CHECK_NAME("/DATA", true);
    CHECK_NAME("/DATA.", true);
    CHECK_NAME("/DATA.TXT", true);
    CHECK_NAME("/DATADATA.TXT", true);
    CHECK_NAME("/DATADATA.TXT", true);
    CHECK_NAME("/DATADATA.T", true);
    CHECK_NAME("/DATADATA.", true);
    CHECK_NAME("/", false);
    CHECK_NAME("/DATADATA.TXTX", false);
    CHECK_NAME("/DATADATAD", false);
    CHECK_NAME("/DATADATAD.", false);
    CHECK_NAME("/DATADATAD.TXTX", false);

    return true;
}

bool FilenameTest::check_name(const std::string &name, bool expected_to_succeed)
{
    int ret = fat16_open(name.c_str(), 'w');
    if (ret == 0)
        fat16_close(ret);

    if (expected_to_succeed && ret != 0)
        return false;
    if (!expected_to_succeed && ret == 0)
        return false;
    return true;
}
