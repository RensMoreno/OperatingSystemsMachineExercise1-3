// Compile: gcc MachineExercise2b.c -o MachineExercise2b
// Compile child: gcc MachineExercise2bChild.c -o MachineExercise2bChild
// Run: ./MachineExercise2b

#include <errno.h>
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
        execl("./MachineExercise2bChild", "MachineExercise2bChild",
              (char *)NULL);
        perror("execl");                     /* only reached if exec FAILED */
        _exit(EXIT_FAILURE);
    }

    /* parent */
    printf("[PARENT]: PID %ld, waits for child with PID %ld\n",
           (long)getpid(), (long)pid);
    fflush(stdout);

    int status;
    pid_t waited_pid;
    do {
        waited_pid = waitpid(pid, &status, 0);
    } while (waited_pid == -1 && errno == EINTR);

    if (waited_pid == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        printf("[PARENT]: Child with PID %ld finished and unloaded (exit code %d).\n",
               (long)pid, WEXITSTATUS(status));
        return WEXITSTATUS(status) == EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    if (WIFSIGNALED(status)) {
        fprintf(stderr, "[PARENT]: Child with PID %ld terminated by signal %d.\n",
                (long)pid, WTERMSIG(status));
    } else {
        fprintf(stderr, "[PARENT]: Child with PID %ld did not exit normally.\n",
                (long)pid);
    }
    return EXIT_FAILURE;
}