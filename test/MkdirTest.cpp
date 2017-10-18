#include "Common.hpp"
#include "MkdirTest.hpp"
#include "../driver/fat16.h"
#include "linux_hal.h"


MkdirTest::MkdirTest():
Test("MkdirTest")
{

}

bool MkdirTest::run()
{
    if (fat16_init(linux_dev) < 0)
        return false;

    if (fat16_mkdir("/DATA") < 0)
        return false;
    if (fat16_open("/DATA", 'r') == 0)
        return false;
    if (fat16_mkdir("/TMP") < 0)
        return false;
    if (fat16_open("/TMP", 'w') == 0)
        return false;

    if (fat16_mkdir("/IMAGES") < 0)
        return false;
    if (fat16_mkdir("/IMAGES/JPEG") < 0)
        return false;
    if (fat16_mkdir("/IMAGES/PNG") < 0)
        return false;
    if (fat16_mkdir("/IMAGES/BMP") < 0)
        return false;

    return true;
}
