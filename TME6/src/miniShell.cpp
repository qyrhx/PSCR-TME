#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

// Custom mystrdup : allocates with new[], copies string (avoid strdup and free)
char *mystrdup(const char *src);

pid_t child = 0;
void kill_child(int sig) {
  if (child == 0)
    return;
  kill(child, SIGINT);
  const char *msg = "-- Sent SIGINT to child\n";
  write(STDOUT_FILENO, msg, strlen(msg));
}

int wait_till_pid(pid_t p, unsigned sec) {
  int status = 0;
  if (sec != 0)
    alarm(sec);
  while (wait(&status) != p)
    ;
  std::string msg;
  if (WIFSIGNALED(status)) {
    msg = std::format("Child {} exited because of a signal: {} ({})\n", p, WTERMSIG(status),
                      strsignal(WTERMSIG(status)));
  } else if (WIFEXITED(status)) {
    msg = std::format("Child {} exited normally with status code: {}\n", p, WEXITSTATUS(status));
  }
  write(STDOUT_FILENO, msg.c_str(), msg.length());
  return p;
}

int main() {
  std::string line;

  printf("Built-in commands:\n"
         "> exit\n"
         "> :t   -> get current value of timeout\n"
         "> :t N -> set timeout (0 = no timeout)\n");

  struct sigaction sa = {0};
  sa.sa_handler = kill_child;
  sigemptyset(&sa.sa_mask);
  // SA_RESTART, pour que le handler restarte std::getline
  // quand on fait Ctrl-C, sinon, il retourne avec une erreur
  // => se comporte comme si on a fait Ctrl-D
  sa.sa_flags = SA_RESTART;
  sigaction(SIGINT, &sa, NULL);

  // Timeout
  sigaction(SIGALRM, &sa, NULL);

  unsigned timeout = 0;
  while (true) {
    std::cout << "mini-shell> " << std::flush;
    if (!std::getline(std::cin, line)) { // SA_RESTART
      std::cout << "\nExiting on EOF (Ctrl-D)." << std::endl;
      break;
    }

    if (line.empty())
      continue;

    // Simple parsing: split by spaces using istringstream
    std::istringstream iss(line);
    std::vector<std::string> args;
    std::string token;
    while (iss >> token) {
      args.push_back(token);
    }
    if (args.empty())
      continue;

    // Prepare C-style argv: allocate and mystrdup
    char **argv = new char *[args.size()];
    for (size_t i = 0; i < args.size(); ++i) {
      argv[i] = mystrdup(args[i].c_str());
    }

    // Your code for fork/exec/wait/signals goes here...
    if (args[0] == "exit")
      exit(0);
    else if (args[0] == ":t") {
      if (args.size() == 1) {
        std::cout << "Current timeout value: " << timeout << '\n';
        continue;
      }
      try {
        timeout = std::stoul(args[1]);
        if (timeout == 0)
          std::cout << "Timeout disabled\n";
        else
          std::cout << "Timeout set to: " << timeout << " seconds \n";
      } catch (const std::invalid_argument &e) {
        std::cout << "Error: invalid argument\n";
      } catch (const std::out_of_range &e) {
        std::cout << "Error: timeout value is too large\n";
      }
      continue;
    }
    child = fork();
    if (child == 0) {
      if (execvp(argv[0], argv) == -1)
        exit(1);
    } else {
      wait_till_pid(child, timeout);
      child = 0;
    }

    // cleanup argv allocations
    for (size_t i = 0; i < args.size(); ++i) {
      delete[] argv[i];
    }
    delete[] argv;
  }
  return 0;
}

char *mystrdup(const char *src) {
  if (src == nullptr)
    return nullptr;
  size_t len = strlen(src) + 1; // +1 for null terminator
  char *dest = new char[len];
  memcpy(dest, src, len); // Or strcpy if preferred
  return dest;
}
