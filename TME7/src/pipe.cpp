#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {

  // Parse command line arguments: find the pipe separator "|"
  // Format: ./pipe cmd1 [args...] | cmd2 [args...]
  int e2i = -1;
  for (int i = 1; i < argc; ++i) {
    if (0 == strcmp(argv[i], "|")) {
      argv[i] = 0; // remplacer la "|" par NULL
      e2i = i + 1;
      break;
    }
  }
  // Create a pipe for inter-process communication
  int fd[2];
  pipe(fd);

  // Fork the first child process ; child redirects out to write end of pipe, then exec
  pid_t c1 = fork();
  if (c1 == 0) {
    dup2(fd[1], STDOUT_FILENO);
    if (close(fd[0]) == -1) {
      perror("close");
    }
    if (close(fd[1]) == -1) {
      perror("close");
    }
    if (-1 == execvp(argv[1], argv + 1)) {
      perror("execvp");
    }
  }
  // Fork the second child process ; child redirects in from read end of pipe, then exec
  pid_t c2 = fork();
  if (c2 == 0) {
    dup2(fd[0], STDIN_FILENO);
    if (close(fd[1]) == -1) {
      perror("close");
    }
    if (close(fd[0]) == -1) {
      perror("close");
    }
    if (-1 == execvp(argv[e2i], argv + e2i)) {
      perror("execvp");
    }
  }
  close(fd[0]);
  close(fd[1]);
  // Wait for both children to finish
  wait(0);
  wait(0);
}
