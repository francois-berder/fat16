#ifndef _FILENAMETEST_HPP_
#define _FILENAMETEST_HPP_

#include "Test.hpp"

class FilenameTest : public Test
{
    public:

        FilenameTest();

        virtual bool run() override;

    private:
        bool check_name(const std::string &name, bool expected_to_succeed);
};

#endif
