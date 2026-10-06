/* ME2-B (parent program): fork(), exec the child binary, wait() for it.
 * Compile: gcc me2b.c -o me2b   (and build ./counter first, see me2b_child.c)
 * Run:     ./me2b
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {                          /* child: replace its code */
        execl("./counter", "counter", (char *)NULL);
        perror("execl");                     /* only reached if exec FAILED */
        _exit(EXIT_FAILURE);
    }

    /* parent */
    printf("[PARENT]: PID %d, waits for child with PID %d\n", getpid(), pid);
    fflush(stdout);

    int status;
    if (wait(&status) == -1) {               /* sleeps: no CPU, no I/O */
        perror("wait");
        return EXIT_FAILURE;
    }
    if (WIFEXITED(status))
        printf("[PARENT]: Child with PID %d finished and unloaded (exit code %d).\n",
               pid, WEXITSTATUS(status));
    return 0;
}