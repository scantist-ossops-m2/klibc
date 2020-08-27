/*
 * sigaction.c
 */

#include <signal.h>
#include <stddef.h>
#include <sys/syscall.h>
#include <klibc/sysconfig.h>

__extern void __sigreturn(void);
__extern int __sigaction(int, const struct sigaction *, struct sigaction *);
#ifdef __sparc__
__extern int __rt_sigaction(int, const struct sigaction *, struct sigaction *,
			    void (*)(void), size_t);
#elif defined(__alpha__)
__extern int __rt_sigaction(int, const struct sigaction *, struct sigaction *,
			    size_t, void *);
#else
__extern int __rt_sigaction(int, const struct sigaction *, struct sigaction *,
			    size_t);
#endif

int sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
	int rv;

#if _KLIBC_NEEDS_SA_RESTORER
	struct sigaction sa;

	if (act && !(act->sa_flags & SA_RESTORER)) {
		sa = *act;
		act = &sa;

		/* The kernel can't be trusted to have a valid default
		   restorer */
		sa.sa_flags |= SA_RESTORER;
		sa.sa_restorer = &__sigreturn;
	}
#endif

#if _KLIBC_USE_RT_SIG
	/* Check that we have the right signal API definitions */
	(void)sizeof(char[_NSIG >= 64 ? 1 : -1]);
	(void)sizeof(char[sizeof(sigset_t) * 8 >= _NSIG ? 1 : -1]);
	(void)sizeof(char[offsetof(struct sigaction, sa_mask)
			  + sizeof(sigset_t) == sizeof(struct sigaction)
			  ? 1 : -1]);

# ifdef __sparc__
	{
		void (*restorer)(void);
		restorer = (act && act->sa_flags & SA_RESTORER)
			? (void (*)(void))((uintptr_t)act->sa_restorer - 8)
			: NULL;
		rv = __rt_sigaction(sig, act, oact, restorer, sizeof(sigset_t));
	}
# elif defined(__alpha__)
	rv = __rt_sigaction(sig, act, oact, sizeof(sigset_t), NULL);
# else
	rv = __rt_sigaction(sig, act, oact, sizeof(sigset_t));
# endif
#else
	rv = __sigaction(sig, act, oact);
#endif

#if _KLIBC_NEEDS_SA_RESTORER
	if (oact && (oact->sa_restorer == &__sigreturn)) {
		oact->sa_flags &= ~SA_RESTORER;
	}
#endif

	return rv;
}
