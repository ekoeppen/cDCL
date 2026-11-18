// Standalone death test verifying TRefCounter triggers KASSERT(mRefCount == 0)
// via SIGINT when a TDCLNSArray is destroyed while a TDCLNSRef still references it.

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/NS_Objects/Objects/TDCLNSArray.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
#include <DCL/NS_Objects/Objects/TDCLNSRef.h>

#if TARGET_OS_COMPAT_POSIX
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

int main(void)
{
	{
		TDCLNSFrame someFrame;
		{
			TDCLNSArray someArray;
			someFrame.Set("foo", someArray.Clone());
		} // no assertion
	}

#if TARGET_OS_COMPAT_POSIX && !NDEBUG
	const pid_t pid = ::fork();
	if (pid < 0)
	{
		return 1;
	}
	if (pid == 0)
	{
		TDCLNSFrame someFrame;
		{
			TDCLNSArray someArray;
			someFrame.Set("foo", TDCLNSRef(someArray));
		} // expected KASSERT(mRefCount == 0) -> SIGINT
		_exit(0);
	}

	int status = 0;
	if (::waitpid(pid, &status, 0) != pid)
	{
		return 1;
	}
	return (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT) ? 0 : 1;
#else
	return 0;
#endif
}
