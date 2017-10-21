/*
 * Copyright (C) 2017  Francois Berder
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
#include "LsTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"


LsTest::LsTest(unsigned int files_count):
Test(std::string("LsTest (") + std::to_string(files_count) + std::string(")")),
m_files_count(files_count)
{

}

void LsTest::init()
{
    restore_image();
    mount_image();
    for(unsigned int i = 0; i < m_files_count; ++i) {
        std::stringstream ss;
        ss << "touch /mnt/" << i << ".TXT";
        system(ss.str().c_str());
    }
    unmount_image();

    load_image();
}

bool LsTest::run()
{
    if (fat16_init(linux_dev) < 0)
        return false;

    std::vector<bool> file_exist;
    file_exist.resize(m_files_count);
    for (unsigned int j = 0; j < file_exist.size(); ++j)
        file_exist[j] = false;

    unsigned int files_count = 0;
    char filename[13];
    uint16_t i = 0;
    int ret = 0;
    while((ret = fat16_ls(&i, filename)) == 0) {
        ++files_count;
        printf("Found file: %s\n", filename);

        /* extract file name */
        std::string s(filename);
        size_t lastindex = s.find_last_of(".");
        std::string rawname = s.substr(0, lastindex);
        long index = std::stol(rawname);
        if (index < 0 || index >= m_files_count)
            return false;
        file_exist[index] = true;

        /* Check filename */
        std::string correct_filename = std::to_string(index) + ".TXT";
        if (correct_filename != filename)
            return false;
    }
    if (ret != -2)
        return false;

    for (unsigned int j = 0; j < file_exist.size(); ++j) {
        if (!file_exist[j]) {
            std::cout << "Could not find file \'" << j << ".TXT\'" << std::endl;
            return false;
        }
    }

    return files_count == m_files_count;
}
