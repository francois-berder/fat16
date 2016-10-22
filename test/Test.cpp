#include "Test.hpp"


Test::Test(const std::string &name):
m_name(name)
{
}

Test::~Test()
{
}

void Test::init()
{

}

void Test::release()
{

}

const std::string Test::get_name() const
{
    return m_name;
}
