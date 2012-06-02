
// @Name   : test.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-03-09 00:03:46
//
// @Brief  :

#include <test.h>
#include <asm.h>
#include <cpu.h>
#include <screen.h>
#include <string.h>
#include <page.h>
#include <kheap.h>


int test_all()
{
    test_print();
//    test_detect_cpu();
//    test_page_fault();
    test_kmalloc();
    return 1;
}

int test_print()
{
    printk("value: %d\n", -2);
    printk("value: %d\n", -10);
    printk("value: %d\n", 10);
    printk("value: %d\n", 11);
    printk("value: %d\n", -1);
    printk("value: %d\n", 9);
    printk("value: %d\n", 1<<31);
    printk("value: %d\n", 0x7fffffff);
    return 1;
}

int test_page_fault()
{
    u32 *ptr = (u32*)0x100E000;
    u32 do_page_fault = *ptr;
    printk("value: %d\n", do_page_fault);
    kassert(do_page_fault != 0);
    return 1;
}

int test_detect_cpu()
{
    detect_cpu();
    return 1;
}

int test_kmalloc()
{
    int k;
    for(k=0; k<4; k++) {
        u32 *ptr = (u32*)kmalloc(sizeof(u32)*100);
        *ptr = 0xA;
        kfree(ptr);
        char* cp = (char*)kmalloc(sizeof(char)*100);
        *cp = 'c';
        printk("ptr1:%x ptr2:%x \n",ptr,  cp);
        kfree(cp);
    }

    return 1;
}

