#include <format>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>

#include "util/rsleep.h"

volatile sig_atomic_t pv = 3;

void dec_life(int signum) { pv -= 1; };

void attack(pid_t enemy) {
  struct sigaction sa = {0};
  sa.sa_handler = dec_life;
  sigaction(SIGUSR1, &sa, 0);

  if (kill(enemy, SIGUSR1) == -1) {
    std::cout << "WINNER: " << getpid() << '\n';
    exit(0);
  }
  pr::randsleep();
}

void defense() {
  struct sigaction sa = {0};
  sa.sa_handler = SIG_IGN;
  sigaction(SIGUSR1, &sa, 0);
  pr::randsleep();
}

void combat(pid_t enemy) {
  while (pv) {
    defense();
    attack(enemy);
    std::string msg = std::format("{} PV: {}\n", getpid(), (int)pv);
    write(STDIN_FILENO, msg.c_str(), msg.length());
  }
  exit(1);
}

int main() {
  auto p1 = fork();
  if (p1 == 0) {
    combat(getppid());
    exit(pv == 0 ? 1 : 0);
  }
  combat(p1);
}
