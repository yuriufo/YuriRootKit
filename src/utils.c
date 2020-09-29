#include "utils.h"

//********* START WRITE_PROTECT *********

void disable_write_protection(void)
{
    preempt_disable();
    // 将 CR0 寄存器第16个bit置0
    write_cr0(read_cr0() & (~0x10000));
    preempt_enable();
}

void enable_write_protection(void)
{
    preempt_disable();
    // 将 CR0 寄存器第16个bit置1
    write_cr0(read_cr0() | (0x10000));
    preempt_enable();
}

//********* END WRITE_PROTECT *********

//********* START PAGE_PROTECT *********

void disable_page_protection(void)
{
    write_cr0(read_cr0() & (~0x10000));
}

void enable_page_protection(void)
{
    write_cr0(read_cr0() | 0x10000);
}

//********* END PAGE_PROTECT *********