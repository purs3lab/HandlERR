#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int fd[2], debugopt;

int curl_download(const char *url, const int debug) {
  if ( pipe(fd) != 0) {
    fputs("Could not create pipe\n", stderr);
    exit(1);
  }
}
