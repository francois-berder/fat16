#ifndef _READLARGEFILETEST_HPP_
#define _READLARGEFILETEST_HPP_

#include "Test.hpp"

class ReadLargeFileTest : public Test
{
    public :

        ReadLargeFileTest(unsigned int bytes_count);

        virtual void init() override;
        virtual bool run() override;

    private :

        const unsigned int m_bytes_count;

        void create_large_file(const std::string &filename);
};

#endif
