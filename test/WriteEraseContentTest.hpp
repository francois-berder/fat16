#ifndef _WRITEERASECONTENTTEST_HPP_
#define _WRITEERASECONTENTTEST_HPP_

#include "Test.hpp"

class WriteEraseContentTest : public Test
{
    public :

        WriteEraseContentTest();

        virtual void init() override;
        virtual bool run() override;

    private :

        bool check_file_is_empty();
};

#endif
