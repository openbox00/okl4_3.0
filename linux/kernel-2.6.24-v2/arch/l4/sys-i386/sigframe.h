struct sigframe
{
	char *pretcode;
	int sig;
	struct sigcontext sc;
	/*
	 * We issue fxsave / fxrstor on this and it needs
	 * the data to be 16-aligned	-gl
	 */
	struct _fpstate fpstate __attribute__((__aligned__(16)));
	unsigned long extramask[_NSIG_WORDS-1];
	char retcode[8];
	void *sig_ip;	    /* For Wombat trampoline */
};

struct rt_sigframe
{
	char *pretcode;
	int sig;
	struct siginfo *pinfo;
	void *puc;
	struct siginfo info;
	struct ucontext uc;
	/*
	 * We issue fxsave / fxrstor on this and it needs
	 * the data to be 16-aligned	-gl
	 */
	struct _fpstate fpstate __attribute__((__aligned__(16)));
	char retcode[8];
	void *sig_ip;	    /* For Wombat trampoline */
};

struct restore_sigframe
{
	void *ret_ip;
};

