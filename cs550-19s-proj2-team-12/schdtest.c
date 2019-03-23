#include "types.h"
#include "user.h"

#define P_LOOP_CNT 0x01000000
#define C_LOOP_CNT 0x03000000


void do_parent(void)
{
    volatile int cnt = 0;
    volatile int tmp = 0;

    while(cnt < P_LOOP_CNT)
    {
        tmp += cnt;
        cnt ++;
    }
}


void do_child(void)
{
    volatile int cnt = 0;
    volatile int tmp = 0;

    while(cnt < C_LOOP_CNT)
    {
        tmp += cnt;
        cnt ++;
    }

    exit();
}

void example_test_code()
{
    int pid = 0;

    pid = fork();
    if (pid < 0)
    {
        printf(1, "fork() failed!\n");
        exit();
    }
    else if (pid == 0) // child
    {
        //sleep(100);
        do_child();
    }
    else // parent
    {
        do_parent();
        if (wait() < 0)
        {
            printf(1, "wait() failed!\n");
        }
    }
	
	printf(1, "\n");
}

int
main(int argc, char *argv[])
{
    enable_sched_trace(1);

    /* ---------------- start: add your test code ------------------- */

   // example_test_code();

    int scheduler = 0, with_rg_proc = 0;
    int i = 0, j = 0;
    int tmp = 0;
    struct {
        int pid;
        int priority;
    } pa[CHILD_COUNT];
    int rg_pid = 0;
    int pipe_fd[CHILD_COUNT][2];
    int c = 0;

    printf(1, "==============\n");

    if (argv[1][0] == '0')
    {
        scheduler = 0;
        printf(1, "Using the default xv6 scheduler \n");
    }
    else
    {
        scheduler = 1;
        printf(1, "Using the priority based scheduler, ");

        if (argc < 3)
        {
            usage();
            exit();
        }

        if (argv[2][0] == '0')
        {
            with_rg_proc = 0;
            printf(1, "without runtime-generated process\n");
        }
        else
        {
            with_rg_proc = 1;
            printf(1, "with runtime-generated process\n");
        }
    }

    printf(1, "--------------\n");

    set_sched(scheduler);
    
    for (i = 0; i < CHILD_COUNT; i++)
    {
        if (pipe(pipe_fd[i]) < 0)
        {
            printf(1, "pipe() failed!\n");
            exit();
        }

        pa[i].pid = fork();
        if (pa[i].pid < 0)
        {
            printf(1, "fork() failed!\n");
            exit();
        }
        else if (pa[i].pid == 0) // child
        {  
            #if 1
            close(pipe_fd[i][1]); // close the write end
            read(pipe_fd[i][0], &c, 1);
            #endif

            while(j < LOOP_CNT)
            {
                tmp += j;
                j++;
            }
            
            exit();
        }
        else // parent
        {
            close(pipe_fd[i][0]); // close the read end

            if (scheduler == 0) // RR
            {
                printf(1, "Parent: child (pid=%d) created!\n", 
                       pa[i].pid, pa[i].priority);
            }
            else // priority-based
            {
                if (i == 0 || i == 1)
                {
                    pa[i].priority = 1;
                    set_priority(pa[i].pid, pa[i].priority);
                }
                else if (i == 4 || i == 5)
                {
                    pa[i].priority = 3;
                    set_priority(pa[i].pid, pa[i].priority);
                }
                else 
                {
                    pa[i].priority = 2;
                    //Note: no need to set priority, because it should be set to 2 by the kernel by default
                }

                printf(1, "Parent: child (pid=%d priority=%d) created!\n", 
                       pa[i].pid, pa[i].priority);
            }
        }
    }


    #if 1
    for (i = 0; i < CHILD_COUNT; i++)
    {   
       write(pipe_fd[i][1], "A", 1); // start child-i
    }
    #endif

    for (i = 0; i < CHILD_COUNT; i++)
    {   
        if (with_rg_proc)
        {
            if (i == 4)
            {
                rg_pid = fork();
                if (pa[i].pid < 0)
                {
                    printf(1, "fork() failed!\n");
                    exit();
                }
                else if (rg_pid == 0) // child
                {
                    while(j < LOOP_CNT)
                    {
                        tmp += j;
                        j++;
                    }
                    
                    exit();
                }
                else // parent
                {
                    if (wait() < 0)
                    {
                        printf(1, "wait() on child-%d failed!\n", i);
                    }
                }
            }
        }

        if (wait() < 0)
        {
            printf(1, "wait() on child-%d failed!\n", i);
        }
    }

    printf(1, "==============\n");




    /* ---------------- end: add your test code ------------------- */

    enable_sched_trace(0);
    
    exit();
}
