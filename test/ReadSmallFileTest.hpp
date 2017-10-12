#ifndef _READSMALLFILETEST_HPP_
#define _READSMALLFILETEST_HPP_

#include "Test.hpp"

class ReadSmallFileTest : public Test
{
    public :

        ReadSmallFileTest();

        virtual void init() override;
        virtual bool run() override;

    private :

        void create_small_file(const std::string &filename, const std::string &content);

        std::string m_content;
};

#endif
