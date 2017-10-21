#ifndef _DELETEFILETEST_HPP_
#define _DELETEFILETEST_HPP_

#include "Test.hpp"

class DeleteFileTest : public Test
{
    public:

        DeleteFileTest();

        virtual void init() override;
        virtual bool run() override;

    private:

        bool file_exists(const std::string &filepath);
};

#endif
