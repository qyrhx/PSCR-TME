#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

//** `fd` must be open
void cat_file(int fd) {
  static constexpr size_t BUFFER_SIZE = 4096;
  static char buf[BUFFER_SIZE] = {0};
  size_t l;
  do {
    l = read(fd, buf, BUFFER_SIZE);
    write(STDOUT_FILENO, buf, l);
  } while (l != 0);
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    cat_file(STDIN_FILENO);
  } else {
    for (int i = 1; i < argc; ++i) {
      int fd = open(argv[i], O_RDONLY);
      if (fd == -1) {
        perror("open");
        exit(1);
      }
      cat_file(fd);
      close(fd);
    }
  }
}
