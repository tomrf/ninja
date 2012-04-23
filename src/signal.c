#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "common.h"


void sighandler(int s)
{
  switch(s) {

    case SIGCHLD:
      wait(NULL);
      break;

    case SIGSEGV:
      die("got signal SIGSEGV -- segmentation fault");
      break;

    case SIGINT:
      die("got signal SIGINT -- terminating");
      break;

    case SIGQUIT:
      die("got signal SIGQUIT -- terminating");
      break;

    default:
      LOG("ignored signal: %d", s);

  }
}
