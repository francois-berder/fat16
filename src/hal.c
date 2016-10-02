#include "hal.h"
#include "linux_hal.h"
#include "pic24_hal.h"

int hal_read(uint8_t *buffer, uint32_t length)
{
#ifdef PLATFORM_LINUX
    return linux_read(buffer, length);
#elif defined PLATFORM_PIC24
    return pic24_read(buffer, length);
#else
    return -1;
#endif
}

int hal_read_byte(uint8_t *data)
{
#ifdef PLATFORM_LINUX
    return linux_read_byte(data);
#elif defined PLATFORM_PIC24
    return pic24_read_byte(data);
#else
    return -1;
#endif
}

int hal_seek(int offset)
{
#ifdef PLATFORM_LINUX
    return linux_seek(offset);
#elif defined PLATFORM_PIC24
    return pic24_seek(offset);
#else
    return -1;
#endif
}

int hal_write(uint8_t *buffer, uint32_t length)
{
#ifdef PLATFORM_LINUX
    return linux_write(buffer, length);
#elif defined PLATFORM_PIC24
    return pic24_write(buffer, length);
#else
    return -1;
#endif
}
