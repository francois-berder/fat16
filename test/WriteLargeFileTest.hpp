#ifndef _WRITELARGEFILETEST_HPP_
#define _WRITELARGEFILETEST_HPP_

#include "Test.hpp"

class WriteLargeFileTest : public Test
{
    public :

        WriteLargeFileTest(unsigned int bytes_count);

        virtual void init() override;
        virtual bool run() override;

    private :

        const unsigned int m_bytes_count;

        bool check_content_file(const std::string &filename);
};

#endif
