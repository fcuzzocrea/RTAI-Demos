#include <linux/module.h>

#include <asm/rtai.h>
#include <asm/rtai_fpu.h>

#define IRQ 0

#ifdef CONFIG_SMP
static int vector = 0x31; // SMPwise it is likely 0x31
#else
static int vector = 0x20; // UPwise it is likely 0x20
#endif

extern struct desc_struct idt_table[];

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#define __LXRT_GET_DATASEG(reg) "movl $" STR(__KERNEL_DS) ",%" #reg "\n\t"
#else /* KERNEL_VERSION >= 2.6.0 */
#define __LXRT_GET_DATASEG(reg) "movl $" STR(__USER_DS) ",%" #reg "\n\t"
#endif  /* KERNEL_VERSION < 2.6.0 */

static unsigned long cr0;
static FPU_ENV saved_fpu_reg, my_fpu_reg;
static float fcnt = 0.0;

static void c_handler (void)
{
	static int cnt;
	adp_root->irqs[IRQ].acknowledge(IRQ);
	save_cr0_and_clts(cr0);
	save_fpenv(saved_fpu_reg);
	restore_fpenv(my_fpu_reg);
	fcnt++;
	rt_printk("HEY HERE I AM %d\n", ++cnt);
	save_fpenv(my_fpu_reg);
	restore_fpenv(saved_fpu_reg);
	restore_cr0(cr0);
	rt_pend_linux_irq(IRQ);
}

static void asm_handler (void)
{
    __asm__ __volatile__ ( \
	"cld\n\t" \
        "pushl %es\n\t" \
        "pushl %ds\n\t" \
        "pushl %eax\n\t" \
        "pushl %ebp\n\t" \
	"pushl %edi\n\t" \
        "pushl %esi\n\t" \
        "pushl %edx\n\t" \
        "pushl %ecx\n\t" \
	"pushl %ebx\n\t" \
	__LXRT_GET_DATASEG(ebx) \
        "movl %ebx, %ds\n\t" \
        "movl %ebx, %es\n\t" \
        "call "SYMBOL_NAME_STR(c_handler)"\n\t" \
        "popl %ebx\n\t" \
        "popl %ecx\n\t" \
        "popl %edx\n\t" \
        "popl %esi\n\t" \
	"popl %edi\n\t" \
        "popl %ebp\n\t" \
        "popl %eax\n\t" \
        "popl %ds\n\t" \
        "popl %es\n\t" \
        "iret");
}

struct desc_struct rtai_set_gate_vector (unsigned vector, int type, int dpl, void *handler)
{
	struct desc_struct e = idt_table[vector];
	idt_table[vector].a = (__KERNEL_CS << 16) | ((unsigned)handler & 0x0000FFFF);
	idt_table[vector].b = ((unsigned)handler & 0xFFFF0000) | (0x8000 + (dpl << 13) + (type << 8));
	return e;
}

void rtai_reset_gate_vector (unsigned vector, struct desc_struct e)
{
	idt_table[vector] = e;
}

static struct desc_struct desc;

int xinit_module(void)
{
	unsigned long flags;
	printk("TIMER IRQ/VECTOR %d/%d\n", IRQ, vector);
        save_cr0_and_clts(cr0);
        save_fpenv(my_fpu_reg);
        restore_cr0(cr0);
	flags = adeos_critical_enter(NULL);
	desc = rtai_set_gate_vector(vector, 14, 0, asm_handler);
	adeos_critical_exit(flags);
	return 0;
}

void xcleanup_module(void)
{
	unsigned long flags;
	flags = adeos_critical_enter(NULL);
	rtai_reset_gate_vector(vector, desc);
	adeos_critical_exit(flags);
	printk("TIMER IRQ/VECTOR %d/%d, FPCOUNT %lu\n", IRQ, vector, (unsigned long)fcnt);
	return;
}

module_init(xinit_module);
module_exit(xcleanup_module);
