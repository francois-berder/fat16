#ifndef _DELETEDIRECTORYTEST_HPP_
#define _DELETEDIRECTORYTEST_HPP_

#include "Test.hpp"

class DeleteDirectoryTest : public Test
{
    public:

        DeleteDirectoryTest();

        virtual void init() override;
        virtual bool run() override;

    private:

        bool directory_exists(const std::string &dirpath);
};

#endif
