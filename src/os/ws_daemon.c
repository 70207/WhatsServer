#include <ws_std.h>
#include <fcntl.h>


#include <signal.h>

#include <ws_cycle.h>
#include <ws_daemon.h>
#include <ws_process.h>

ws_int32_t ws_daemon_init();


ws_int32_t ws_daemon()
{
	ws_int32_t flag;

	flag = ws_daemon_init();

	if( flag ){
		exit(0);
	}
	

	return YMZ_DAEMON_OK;
}

ws_int32_t ws_daemon_init()
{
#ifdef __linux__
	pid_t			pid;
	ws_int32_t				fd;

	pid = fork();

	if(pid < 0)
	{
		return YMZ_DAEMON_ERROR;
	}
	else if(pid > 0)
	{
		exit(0);		//parent exit;
	}

	ws_pid = getpid();
	if (setsid() == -1) {
        return YMZ_ERROR;
    }


	//umask( 0 );

	fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        return YMZ_ERROR;
    }

    if (dup2(fd, STDIN_FILENO) == -1) {
		return YMZ_ERROR;
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
        return YMZ_ERROR;
    }

	if (fd > STDERR_FILENO) {
        if (close(fd) == -1) {
            return YMZ_ERROR;
        }
    }

	////child continues
	//setsid();		//become session leader
	//chdir("/");		//change working directory
	//umask(0);		//clear file mode creation mask
	//close(0);		//close stdin
	//close(1);		//close stdout
	//close(2);		//close stderr

#endif
	return YMZ_OK;
}
