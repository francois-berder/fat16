#ifndef _WRITESMALLFILETEST_HPP_
#define _WRITESMALLFILETEST_HPP_

#include "Test.hpp"

class WriteSmallFileTest : public Test
{
    public :

        WriteSmallFileTest();

        virtual bool run() override;

    private :

        bool check_content_file(const std::string &filename, const std::string &content);
};

#endif
