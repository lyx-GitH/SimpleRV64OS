#include "printk.h"
#include "defs.h"

// Please do not modify

void test()
{
    int s = 0;
    while (1)
    {
        s++;
        if (s % 100000007 == 0)
        {
            printk("kernel is running\n");
            s = 0;
        }
    };
}
