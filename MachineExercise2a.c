//Compile: gcc me2a.c -o me2a      Run: ./me2a
//Stop it with Ctrl+C, or from another terminal: kill <pid>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0) {                       /* fork failed */
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {                      /* child */
        while (1) {
            printf("[CHILD]:  PID %d, PPID %d\n", getpid(), getppid());
            fflush(stdout);              /* show output right away */
            sleep(1);                    /* slow down so we can read it */
        }
    } else {                             /* parent */
        while (1) {
            printf("[PARENT]: PID %d, PPID %d\n", getpid(), getppid());
            fflush(stdout);
            sleep(1);
        }
    }
    return 0;
}