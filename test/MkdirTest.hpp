#ifndef _MKDIRTEST_HPP_
#define _MKDIRTEST_HPP_

#include "Test.hpp"

class MkdirTest : public Test
{
    public:

        MkdirTest();

        virtual void init() override;
        virtual bool run() override;
};

#endif
