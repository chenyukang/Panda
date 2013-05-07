
// @Name   : cpu.c 
//
//
// Copyright (c) 2006-2007 -  http://brynet.biz.tm - <brynet@gmail.com>
//
// modify by  Yukang Chen (moorekang@gmail.com) 
// @Date   :  2012-01-07 23:42:55
//
// @Brief  :  detect cpu info


#include <types.h>
#include <cpu.h>
#include <screen.h>
#include <string.h>

/* Required Declarations */
static int do_intel(void);
static int do_amd(void);
void printregs(int eax, int ebx, int ecx, int edx);

#define cpuid(in, a, b, c, d) __asm__("cpuid": "=a" (a), "=b" (b), \
                                      "=c" (c), "=d" (d) : "a" (in));

/* Simply call this function detect_cpu(); */
int detect_cpu(void) { 
	unsigned long ebx, unused;
	cpuid(0, unused, ebx, unused, unused);
	switch(ebx) {
    case 0x756e6547: /* Intel Magic Code */
        do_intel();
		break;
    case 0x68747541: /* AMD Magic Code */
        do_amd();
		break;
    default:
		puts("Unknown x86 CPU Detected\n");
		break;
	}
	return 0;
}

/* Intel Specific brand list */
char *Intel[24] = {
	"Brand ID Not Supported.", 
	"Intel(R) Celeron(R) processor", 
	"Intel(R) Pentium(R) III processor", 
	"Intel(R) Pentium(R) III Xeon(R) processor", 
	"Intel(R) Pentium(R) III processor", 
	"Reserved", 
	"Mobile Intel(R) Pentium(R) III processor-M", 
	"Mobile Intel(R) Celeron(R) processor", 
	"Intel(R) Pentium(R) 4 processor", 
	"Intel(R) Pentium(R) 4 processor", 
	"Intel(R) Celeron(R) processor", 
	"Intel(R) Xeon(R) Processor", 
	"Intel(R) Xeon(R) processor MP", 
	"Reserved", 
	"Mobile Intel(R) Pentium(R) 4 processor-M", 
	"Mobile Intel(R) Pentium(R) Celeron(R) processor", 
	"Reserved", 
	"Mobile Genuine Intel(R) processor", 
	"Intel(R) Celeron(R) M processor", 
	"Mobile Intel(R) Celeron(R) processor", 
	"Intel(R) Celeron(R) processor", 
	"Mobile Geniune Intel(R) processor", 
	"Intel(R) Pentium(R) M processor", 
	"Mobile Intel(R) Celeron(R) processor"
};

/* This table is for those brand strings that have two values
   depending on the processor signature. It should have the same
   number of entries as the above table. */
char *Intel_Other[24] = {
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Intel(R) Celeron(R) processor", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Intel(R) Xeon(R) processor MP", 
	"Reserved", 
	"Reserved", 
	"Intel(R) Xeon(R) processor", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved"
};

/* Intel-specific information */
int do_intel(void) {
	puts("Intel Specific Features:\n");
	unsigned long eax, ebx, ecx, edx, max_eax, signature, unused;
	int model, family, type, brand, stepping, reserved;
	int extended_family = -1;
	cpuid(1, eax, ebx, unused, unused);
	model = (eax >> 4) & 0xf;
	family = (eax >> 8) & 0xf;
	type = (eax >> 12) & 0x3;
	brand = ebx & 0xff;
	stepping = eax & 0xf;
	reserved = eax >> 14;
	signature = eax;
	printk("Type %d - ", type);
	switch(type) {
    case 0:
		puts("Original OEM");
		break;
    case 1:
		puts("Overdrive");
		break;
    case 2:
		puts("Dual-capable");
		break;
    case 3:
		puts("Reserved");
		break;
	}
	puts("\n");
	printk("Family %d - ", family);
	switch(family) {
    case 3:
		puts("i386");
		break;
    case 4:
		puts("i486");
		break;
    case 5:
		puts("Pentium");
		break;
    case 6:
		puts("Pentium Pro");
		break;
    case 15:
		puts("Pentium 4");
	}
	puts("\n");
	if(family == 15) {
		extended_family = (eax >> 20) & 0xff;
		printk("Extended family %d\n", extended_family);
	}
	printk("Model %d - ", model);
	switch(family) {
    case 3:
		break;
    case 4:
		switch(model) {
        case 0:
        case 1:
			puts("DX");
			break;
        case 2:
			puts("SX");
			break;
        case 3:
			puts("487/DX2");
			break;
        case 4:
			puts("SL");
			break;
        case 5:
			puts("SX2");
			break;
        case 7:
			puts("Write-back enhanced DX2");
			break;
        case 8:
			puts("DX4");
			break;
		}
		break;
    case 5:
		switch(model) {
        case 1:
			puts("60/66");
			break;
        case 2:
			puts("75-200");
			break;
        case 3:
			puts("for 486 system");
			break;
        case 4:
			puts("MMX");
			break;
		}
		break;
    case 6:
		switch(model) {
        case 1:
			puts("Pentium Pro");
			break;
        case 3:
			puts("Pentium II Model 3");
			break;
        case 5:
			puts("Pentium II Model 5/Xeon/Celeron");
			break;
        case 6:
			puts("Celeron");
			break;
        case 7:
			puts("Pentium III/Pentium III Xeon - external L2 cache");
			break;
        case 8:
			puts("Pentium III/Pentium III Xeon - internal L2 cache");
			break;
		}
		break;
    case 15:
		break;
	}
	puts("\n");
	cpuid(0x80000000, max_eax, unused, unused, unused);
	/* Quok said: If the max extended eax value is high enough to support the processor brand string
       (values 0x80000002 to 0x80000004), then we'll use that information to return the brand information. 
       Otherwise, we'll refer back to the brand tables above for backwards compatibility with older processors. 
       According to the Sept. 2006 Intel Arch Software Developer's Guide, if extended eax values are supported, 
       then all 3 values for the processor brand string are supported, but we'll test just to make sure and be safe. */
	if(max_eax >= 0x80000004) {
		puts("Brand: ");
		if(max_eax >= 0x80000002) {
			cpuid(0x80000002, eax, ebx, ecx, edx);
			printregs(eax, ebx, ecx, edx);
		}
		if(max_eax >= 0x80000003) {
			cpuid(0x80000003, eax, ebx, ecx, edx);
			printregs(eax, ebx, ecx, edx);
		}
		if(max_eax >= 0x80000004) {
			cpuid(0x80000004, eax, ebx, ecx, edx);
			printregs(eax, ebx, ecx, edx);
		}
		puts("\n");
	} else if(brand > 0) {
		printk("Brand %d - ", brand);
		if(brand < 0x18) {
			if(signature == 0x000006B1 || signature == 0x00000F13) {
				printk("%s\n", Intel_Other[brand]);
			} else {
				printk("%s\n", Intel[brand]);
			}
		} else {
			puts("Reserved\n");
		}
	}
	printk("Stepping: %d Reserved: %d\n", stepping, reserved);
	return 0;
}

/* Print Registers */
void printregs(int eax, int ebx, int ecx, int edx) {
	int j;
	char string[17];
	string[16] = '\0';
	for(j = 0; j < 4; j++) {
		string[j] = eax >> (8 * j);
		string[j + 4] = ebx >> (8 * j);
		string[j + 8] = ecx >> (8 * j);
		string[j + 12] = edx >> (8 * j);
	}
	puts(string);
}

/* AMD-specific information */
int do_amd(void) {
	puts("AMD Specific Features:\n");
	unsigned long extended, eax, ebx, ecx, edx, unused;
	int family, model;
	cpuid(1, eax, unused, unused, unused);
	model = (eax >> 4) & 0xf;
	family = (eax >> 8) & 0xf;

    cpuid(0x80000000, extended, unused, unused, unused);
	if(extended == 0) {
		return 0;
	}
	if(extended >= 0x80000002) {
		unsigned int j;
		puts("Processor Name: ");
		for(j = 0x80000002; j <= 0x80000004; j++) {
			cpuid(j, eax, ebx, ecx, edx);
			printregs(eax, ebx, ecx, edx);
		}
		puts("\n");
	}
	if(extended >= 0x80000007) {
		cpuid(0x80000007, unused, unused, unused, edx);
		if(edx & 1) {
			puts("Temperature Sensing Diode Detected!\n");
		}
	}

	printk("Family: %d Model: %d [", family, model);
	switch(family) {
    case 4:
		printk("486 Model %d", model);
		break;
    case 5:
		switch(model) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
        case 7:
			printk("K6 Model %d", model);
			break;
        case 8:
			puts("K6-2 Model 8");
			break;
        case 9:
			puts("K6-III Model 9");
			break;
        default:
			printk("K5/K6 Model %d", model);
			break;
		}
		break;
    case 6:
		switch(model) {
        case 1:
        case 2:
        case 4:
			printk("Athlon Model %d", model);
			break;
        case 3:
			puts("Duron Model 3");
			break;
        case 6:
			puts("Athlon MP/Mobile Athlon Model 6");
			break;
        case 7:
			puts("Mobile Duron Model 7");
			break;
        default:
			printk("Duron/Athlon Model %d", model);
			break;
		}
		break;
    default:
        puts("Unknown model");
        break;
	}
	puts("]\n");
	return 0;
}
