#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "../driver/fat16.h"
#include "ReadEmptyFileTest.hpp"
#include "ReadSmallFileTest.hpp"
#include "WriteEraseContentTest.hpp"
#include "WriteSmallFileTest.hpp"
#include "WriteLargeFileTest.hpp"
#include "ReadLargeFileTest.hpp"

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

    for (Test *test : tests) {
        std::cout << "===== " << test->get_name() << " =====" << std::endl;
        test->init();
        bool result = test->run();
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

    return 0;
}
