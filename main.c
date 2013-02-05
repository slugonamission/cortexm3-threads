#include "usbcon.h"
#include "threads.h"

void test()
{
    while(1)
    {
        usbcon_write("THREAD LOL\r\n");
    }
}

void test2()
{
    while(1)
    {
        usbcon_write("Thread2\r\n");
    }
}

int main()
{
    usbcon_init();
    usbcon_write("Hello World!\r\n");

    thread_init(10000);
    if(thread_create(test) == -1)
        usbcon_write("Thread creation failed\r\n");;

    if(thread_create(test2) == -1)
        usbcon_write("Thread creation failed\r\n");;
    thread_go();
    
    while(1); // Just incase

    return 0;
}
