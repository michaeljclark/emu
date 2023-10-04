#include "stdio.h"
#include "console.h"

int putchar(int ch)
{
    return console_dev->putchar(ch);
}
