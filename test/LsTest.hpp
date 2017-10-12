#ifndef _LSTEST_HPP_
#define _LSTEST_HPP_

#include "Test.hpp"

class LsTest : public Test
{
    public :

        LsTest(unsigned int files_count);

        virtual void init() override;
        virtual bool run() override;

    private :

        void fill_root_directory();

        const unsigned int m_files_count;
};

#endif
