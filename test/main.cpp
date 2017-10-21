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


#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "../driver/fat16.h"
#include "AppendSmallFileTest.hpp"
#include "FilenameTest.hpp"
#include "ReadEmptyFileTest.hpp"
#include "ReadSmallFileTest.hpp"
#include "WriteEraseContentTest.hpp"
#include "WriteSmallFileTest.hpp"
#include "WriteLargeFileTest.hpp"
#include "ReadLargeFileTest.hpp"
#include "LsTest.hpp"
#include "MkdirTest.hpp"
#include "DeleteFileTest.hpp"
#include "DeleteDirectoryTest.hpp"
#include "Common.hpp"

#define SECTOR_SIZE (2048)

static void print_pass_fail(bool result)
{
    if (result)
        std::cout << "PASS" << std::endl;
    else
        std::cout << "FAIL" << std::endl;
}

int main()
{
    std::vector<Test*> tests;
    std::vector<bool> test_results;
    tests.push_back(new AppendSmallFileTest());
    tests.push_back(new DeleteFileTest());
    tests.push_back(new DeleteDirectoryTest());
    tests.push_back(new FilenameTest());
    tests.push_back(new ReadEmptyFileTest());
    tests.push_back(new ReadSmallFileTest());
    tests.push_back(new WriteEraseContentTest());
    tests.push_back(new WriteSmallFileTest());
    tests.push_back(new WriteLargeFileTest(SECTOR_SIZE/2));
    tests.push_back(new WriteLargeFileTest(SECTOR_SIZE-1));
    tests.push_back(new WriteLargeFileTest(SECTOR_SIZE));
    tests.push_back(new WriteLargeFileTest(SECTOR_SIZE+1));
    tests.push_back(new WriteLargeFileTest(2*SECTOR_SIZE+1));
    tests.push_back(new WriteLargeFileTest(10*SECTOR_SIZE+1));
    tests.push_back(new ReadLargeFileTest(SECTOR_SIZE/2));
    tests.push_back(new ReadLargeFileTest(SECTOR_SIZE-1));
    tests.push_back(new ReadLargeFileTest(SECTOR_SIZE));
    tests.push_back(new ReadLargeFileTest(SECTOR_SIZE+1));
    tests.push_back(new ReadLargeFileTest(2*SECTOR_SIZE+1));
    tests.push_back(new ReadLargeFileTest(10*SECTOR_SIZE+1));
    tests.push_back(new LsTest(0));
    tests.push_back(new LsTest(1));
    tests.push_back(new LsTest(128));
    tests.push_back(new LsTest(256));
    tests.push_back(new LsTest(511));
    tests.push_back(new LsTest(512));
    tests.push_back(new MkdirTest());

    /* Ensure that we start with a clean image */
    unmount_image();
    restore_image();

    unsigned int failing_test_count = 0;
    for (Test *test : tests) {
        std::cout << "===== " << test->get_name() << " =====" << std::endl;
        test->init();
        bool result = test->run();
        if (!result)
            ++failing_test_count;

        test_results.push_back(result);
        print_pass_fail(result);
        test->release();
    }

    // Test result recap
    std::cout << "\n\nTest results:" << std::endl;
    for (unsigned int i = 0; i < tests.size(); ++i) {
        std::cout << std::setw(30) << std::left << tests[i]->get_name();
        std::cout << std::setw(10) << std::left;
        print_pass_fail(test_results[i]);
    }

    for (Test *test : tests)
        delete test;

    return failing_test_count;
}
