#include "usbcon.h"
#include "threads.h"

void test(void* userdata)
{
    while(1)
    {
        usbcon_write((char*)userdata);
    }
}

void test2(void* userdata)
{
    while(1)
    {
        usbcon_write((char*)userdata);
    }
}

int main()
{
    char* str1 = "Hello World!\r\n";
    char* str2 = "HELLO WORLD\r\n";
    usbcon_init();
    usbcon_write("Hello World!\r\n");

    thread_init(10000);
    if(thread_create(test, (void*)str1) == -1)
        usbcon_write("Thread creation failed\r\n");;

    if(thread_create(test2, (void*)str2) == -1)
        usbcon_write("Thread creation failed\r\n");;
    thread_go();
    
    while(1); // Just incase

    return 0;
}
