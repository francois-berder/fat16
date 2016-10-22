#include <iomanip>
#include <iostream>
#include <vector>
#include "../driver/fat16.h"
#include "ReadEmptyFileTest.hpp"

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
        std::cout << std::setw(20) << std::left << tests[i]->get_name();
        std::cout << std::setw(10) << std::left;
        print_pass_fail(test_results[i]);
    }

    for (Test *test : tests)
        delete test;

    return 0;
}
