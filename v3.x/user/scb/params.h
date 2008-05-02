#define BUFSIZE     139
#define SLEEP_TIME  50000

static float randu(void)
{
	static int i = 783637;
	i = 125*i;
	i = i - (i/2796203)*2796203;
	return i/2796203.0;
}
