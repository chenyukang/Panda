
// @Name   : test.c
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-03-09 00:03:46
//
// @Brief  :

#include <test.h>
#include <asm.h>
#include <screen.h>
#include <string.h>
#include <mm.h>


int test_all() {
    test_print();
    test_page_fault();
    return 1;
}

int test_print() {
    printk("value: %d\n", -2);
    printk("value: %d\n", -10);
    printk("value: %d\n", 10);
    printk("value: %d\n", 11);
    printk("value: %d\n", -1);
    printk("value: %d\n", 9);
    printk("value: %d\n", 1<<31);
    printk("value: %d\n", 0x7fffffff);
    printk("value: %s\n", "hello world");
    printk("value: %c\n", 'k');
    printk("value: %x\n", 0xA);
    return 1;
}

int test_page_fault() {
    u32 *ptr = (u32*)0x0;
    //u32* ptr = (u32*)0x0;
    u32 do_page_fault = *ptr;
    printk("value: %d\n", do_page_fault);
    //kassert(do_page_fault != 0);
    *ptr = 0x10;
    do_page_fault = *ptr;
    printk("value: %d\n", do_page_fault);
    u32 k;
    for(k=0; k<0x1000000; k++) {
        u32* addr = (u32*)k;
        printk("addr: %x ==> %x\n", addr, *addr);
        //*addr = 0;
    }
    return 1;
}
