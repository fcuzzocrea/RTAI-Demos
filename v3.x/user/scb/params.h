#define BUFSIZE     139
#define SLEEP_TIME  100000

#ifdef __KERNEL__
static unsigned long randu(unsigned long range)
{
	static unsigned long i = 783637;
	i = 125*i;
	i = i - (i/2796203)*2796203;
	return (i*range)/2796203;
}
#else
static float randu(void)
{
	static unsigned long i = 783637;
	i = 125*i;
	i = i - (i/2796203)*2796203;
	return i/2796203.0;
}
#endif
