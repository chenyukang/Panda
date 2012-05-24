
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
    test_detect_cpu();
    test_page_fault();
    test_kmalloc();
    return 1;
}

int test_print()
{
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
    u32 *ptr = (u32*)kmalloc(sizeof(u32));
    printk("ptr: %x\n", ptr);
    return 1;
}

