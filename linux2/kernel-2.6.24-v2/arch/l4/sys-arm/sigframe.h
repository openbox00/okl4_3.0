/*
 * Do a signal return; undo the signal stack.
 */
struct sigframe
{
	struct sigcontext sc;
	unsigned long extramask[_NSIG_WORDS-1];
	unsigned long retcode;
	void *sig_ip;		/* For Wombat trampoline */
	unsigned long lr;
	int usig;
};

struct rt_sigframe
{
	struct siginfo *pinfo;
	void *puc;
	struct siginfo info;
	struct ucontext uc;
	unsigned long retcode; 
	void *sig_ip;		/* For Wombat trampoline */
	unsigned long lr;
	int usig;
};

struct restore_sigframe
{
	void *ret_ip;
};

