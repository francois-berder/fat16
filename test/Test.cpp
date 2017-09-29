#include <stdexcept>
#include "Common.hpp"
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
    restore_image();
    load_image();
}

void Test::release()
{
    release_image();
}

const std::string Test::get_name() const
{
    return m_name;
}