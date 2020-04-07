
#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "gpio.h"

void uDelay (const uint32_t usec)
{
    uint32_t count = 0;
    const uint32_t utime = (120 * usec / 7);

    do
    {
        if( ++count > utime )
        {
            return ;
        }
    } while (1);
}

void testAllocation(size_t size)
{
    printf("Allocating a chunk of %zd bytes... ");
    uint8_t *p = ((uint8_t *) malloc(size));
    if(p != NULL)
    {
        puts("OK");
        free(p);
    }
    else
    {
        puts("FAIL");
    }
}

int main (void)
{
    size_t chunk = 8;
    while(chunk <= 4096)
    {
        testAllocation(chunk);
        chunk *= 2;
    }

    puts("End of test");
    for(;;) ;
}
