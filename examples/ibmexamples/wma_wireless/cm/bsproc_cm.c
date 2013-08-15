#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <syslog.h>
#include <assert.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <limits.h>

#define BUF_SIZE 	(256)
//#define SHELL_CMD       ("sh ")
#define KILL_CMD	("killall -2 ")
#define BK_CMD		(" &")
#define MAX_CMD_SIZE	(256)
#define LINES_THRESHOLD 500
#define INTERVAL_THRESHOLD 5 //in second

struct kthread_ctx {
	char *kill_cmd;
        FILE *ofd;
};

struct err_item {
	char *err_msg;
	int  occur;
	int  max;
};

struct err_item g_detect_errs[] = { {"9 times", 0, 10},
                                    {"Connection RRH failed", 0, 1},
                                    {"Re-Syncing", 0, 10},
                     		  };
#define ERR_MAX	(3) //keep equal to items in g_detect_errs

pthread_t g_thread_out;

static pid_t
proc_popen( char *argv[], FILE **out)
{
	pid_t pid =-1;
	int fd[2];
	int ret;


	if (pipe(fd) == 0) {
		switch (pid = fork()) {
			case -1:                /* Error. */
				(void) close(fd[0]);
				(void) close(fd[1]);
				break;
			case 0:         /* child. */
				if (fd[1] != STDOUT_FILENO) {
					(void) dup2(fd[1], STDOUT_FILENO);
					(void) close(fd[1]);
				}
				(void) dup2(STDOUT_FILENO, STDERR_FILENO);

				(void) close(fd[0]);

                                ret = execvp(argv[0], argv);
				if( ret == -1 )
                                {
                                	perror( "exec program failed" );
                                }

				_exit(127);
				/* NOTREACHED */
			default:                /* parent */
				*out = fdopen(fd[0], "r");
				(void) close(fd[1]);

				break;
		}
	}

	return pid;
}

void *process_out (void *arg )
{
	char buffer[BUF_SIZE];
	char **p_errmsg;
	char *p_beg;
        ssize_t bytes;

	struct kthread_ctx *ctx =  (struct kthread_ctx *)arg;
	int count = 0;
        int total, ret;
        struct timeval start_t, end_t;
        int i;

	while(1)
	{

		if( count == 0 )
			gettimeofday (&start_t, NULL);



		if( ctx->ofd )
		{

			bytes = read( fileno(ctx->ofd), buffer, sizeof buffer );

			if( bytes > 0 )
			{
				fwrite( buffer, bytes, 1, stdout );
				fflush(stdout);
				count++;

				for( i = 0; i < ERR_MAX; i++)
				{
					p_beg = buffer;
					while((p_beg - buffer) < bytes)		
					{
						p_beg = memchr( p_beg, *g_detect_errs[i].err_msg, bytes + buffer - p_beg );
						if( p_beg )
						{
							if(!strncmp(p_beg, g_detect_errs[i].err_msg, strlen(g_detect_errs[i].err_msg)))
							{
								if( g_detect_errs[i].occur++ >= g_detect_errs[i].max )
								{

									syslog(LOG_INFO, " find %s, restart bsproc... ", *p_errmsg);
									printf(" find %s, restart bsproc... ", *p_errmsg);
									goto kill_cmd;
								}
							}

						}
						else
							break;

						p_beg++;
					}

				} 
			}
			else
			{
				sleep(1);
			}
		}
		else
		{
			sleep(1);
		}

		if( count > LINES_THRESHOLD )
		{
			gettimeofday (&end_t, NULL);
			total = end_t.tv_sec - start_t.tv_sec;

			if( total < INTERVAL_THRESHOLD )
			{
				syslog(LOG_INFO, "find infinite loop, restart bsproc... ");
				goto kill_cmd;
			}

			count = 0;
		} 		

		continue;
kill_cmd:
		ret = system (ctx->kill_cmd);
		if (ret < 0)
		{
			syslog (LOG_INFO, "error in execute: %s", ctx->kill_cmd);
		}
		ctx->ofd = NULL;
	}

}

int daemon_init (void)
{
    pid_t pid;

    pid = fork ();
    if (pid < 0)
    {
        return ( -1 );
    }
    else
        if (pid != 0)
        {
            exit (0); /* parent exit */
        }
        else
        {
            /* child continues */
            setsid (); /* become session leader */
            chdir ("/"); /* change working directory */
            umask (0); /* clear file mode creation mask */
            close (0); /* close stdin */
            close (1); /* close stdout */
            close (2); /* close stderr */
            return ( 0 );
        }
}


void sig_term (int signo)
{
    int status;

    if (signo == SIGTERM)
    {
        /* catched signal sent by kill(1) command */
        syslog (LOG_INFO, "bs monitor program terminated.");
        closelog ();
        wait( &status );
        exit ( status );
    }
}




int main (int argc, char ** argv)
{
    uint32_t interval = 1;
    int ret;
    pid_t bspid;
    FILE *bsout;
    char *call_argv[32];
    int i;
    char kill_cmd[MAX_CMD_SIZE];
    struct kthread_ctx kth_ctx;

    if (argc < 4)
    {
        printf (
                "Usage : %s <wait time> <bsproc path> <bsproc args> \n",
                argv[0]);
        exit (0);
    }

//    if (daemon_init () < 0)
//    {
//        printf ("can’t fork self\n");
//        exit (0);
//    }

    strncpy (kill_cmd, KILL_CMD, sizeof ( kill_cmd ));
    strncat (kill_cmd, argv[1], sizeof ( kill_cmd ) - strlen (kill_cmd));
    kth_ctx.kill_cmd = kill_cmd;
    kth_ctx.ofd = NULL;

    sscanf (argv[2], "%d", &interval);

    openlog ("bs_monitor", LOG_PID, LOG_USER);
    syslog (LOG_INFO, "bs monitor program started.");
    syslog (LOG_INFO, "wait interval = %d.", interval);
    signal (SIGTERM, sig_term); /* arrange to catch the signal */



    for( i = 0; i < argc - 3; i++)
    {
    	call_argv[i] = argv[i + 3];
    }
    call_argv[i] = 0;

    ret = pthread_create (&g_thread_out, NULL, process_out, &kth_ctx);
    if( ret )
    {
    	syslog (LOG_ERR, "create output processing thread error");
    }

    while (1)
    {
	    if ((bspid = proc_popen ( call_argv, &bsout)) > 0)
	    {
		
    		    kth_ctx.ofd = bsout;

    		    for( i = 0; i < ERR_MAX; i++)
		    {
			g_detect_errs[i].occur = 0;
		    }
		    /* execute the scripts */
		    waitpid (bspid, &ret, 0);
    		    kth_ctx.ofd = NULL;
                    ret = ( ret >> 8 ) & 0xFF;
		    syslog (LOG_INFO, "bsproc exit rc=: %d", ret);
	    }
            else
	    {
		    ret = bspid;
		    syslog (LOG_INFO, "start bsproc failed rc=: %d", ret);
                    break;
            }

	    sleep (interval);
    }

    return ret;
}
