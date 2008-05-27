#include <asm/io.h>

#include <linux/mc146818rtc.h>

#if 1

static inline unsigned char RT_CMOS_READ(unsigned char addr)
{
	outb_p(addr, RTC_PORT(0));
	return inb_p(RTC_PORT(1));
}

#else

#define RT_CMOS_READ  CMOS_READ

#endif

//#define TEST_RTC
#define MIN_RTC_FREQ  2
#define MAX_RTC_FREQ  8192
#define RTC_FREQ      MAX_RTC_FREQ

static inline void rtc_enable_irq(int irq, int rtc_freq)
{
#ifdef TEST_RTC
	static int stp, cnt;
	if (++cnt == rtc_freq) {
		rt_printk("<> IRQ %d, %d: CNT %d <>\n", irq, ++stp, cnt);
		cnt = 0;
	}
#endif
 	RT_CMOS_READ(RTC_INTR_FLAGS);
	rt_enable_irq(RTC_IRQ);
}

void rtc_start(long rtc_freq)
{
	int pwr2;

	if (rtc_freq <= 0) {
		rtc_freq = RTC_FREQ;
	}
	if (rtc_freq > MAX_RTC_FREQ) {
		rtc_freq = MAX_RTC_FREQ;
	} else if (rtc_freq < MIN_RTC_FREQ) {
		rtc_freq = MIN_RTC_FREQ;
	}
	pwr2 = 1;
	if (rtc_freq > MIN_RTC_FREQ) {
		while (rtc_freq > (1 << pwr2)) {
			pwr2++;
		}
		if (rtc_freq <= ((3*(1 << (pwr2 - 1)) + 1)>>1)) {
			pwr2--;
		}
	}

	rt_disable_irq(RTC_IRQ);
	rtai_cli();
	CMOS_WRITE(CMOS_READ(RTC_FREQ_SELECT), RTC_FREQ_SELECT);
	CMOS_WRITE(CMOS_READ(RTC_CONTROL),     RTC_CONTROL);
	CMOS_WRITE(RTC_REF_CLCK_32KHZ | (16 - pwr2),          RTC_FREQ_SELECT);
	CMOS_WRITE((CMOS_READ(RTC_CONTROL) & 0x8F) | RTC_PIE, RTC_CONTROL);
	rtai_sti();
	rt_enable_irq(RTC_IRQ);
	CMOS_READ(RTC_INTR_FLAGS);
	return;
}

void rtc_stop(void)
{
	rt_disable_irq(RTC_IRQ);
	rtai_cli();
	CMOS_WRITE(CMOS_READ(RTC_FREQ_SELECT), RTC_FREQ_SELECT);
	CMOS_WRITE(CMOS_READ(RTC_CONTROL),     RTC_CONTROL);
	rtai_sti();
	return;
}
