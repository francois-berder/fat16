#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include "Common.hpp"
#include "LsTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"


LsTest::LsTest(unsigned int files_count):
Test(std::string("LsTest (") + std::to_string(files_count) + std::string(")")),
m_files_count(files_count)
{

}

void LsTest::init()
{
    restore_image();
    mount_image();
    for(unsigned int i = 0; i < m_files_count; ++i) {
        std::stringstream ss;
        ss << "touch /mnt/" << i << ".TXT";
        system(ss.str().c_str());
    }
    unmount_image();

    load_image();
}

bool LsTest::run()
{
    if (fat16_init(linux_dev) < 0)
        return false;

    unsigned int files_count = 0;
    char filename[13];
    uint16_t i = 0;
    int ret = 0;
    while((ret = fat16_ls(&i, filename)) == 0) {
        ++files_count;
        printf("Found file: %s\n", filename);
    }
    if (ret != -2)
        return false;

    return files_count == m_files_count;
}
