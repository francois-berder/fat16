#ifndef _TEST_HPP_
#define _TEST_HPP_

#include <string>

class Test
{
    public :

        Test(const std::string &name);
        virtual ~Test() = default;

        virtual void init();
        virtual bool run() = 0;
        virtual void release();

        const std::string get_name() const;

    private :

        const std::string m_name;
};

#endif
