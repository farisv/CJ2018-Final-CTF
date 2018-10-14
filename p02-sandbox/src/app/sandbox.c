/*
 * Ptrace Sandbox (Syscall Filter)
 * Cyber Jawara 2018 Final - Attack & Defense CTF
 *
 * $ gcc sandbox.c -o sandbox
 * $ ./sandbox [target]
*/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>

// x64 Syscall Number
#define EXECVE 59
#define EXECVEAT 322


void init()
{
  char buff[1];
  buff[0] = 0;
  setvbuf(stdout, buff, _IOFBF, 1);
}

void run_target(const char* programname)
{
  if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
  {
    fprintf(stderr, "Error ptrace\n");
    return;
  }

  execl(programname, programname, NULL);
}

void kill_process(pid_t child_pid)
{
  // Graceful exit
  kill(child_pid, SIGINT);
  sleep(0);
  kill(child_pid, 9);
}

void run_debugger(pid_t child_pid)
{
  pid_t ret;
  struct user_regs_struct regs;
  int status, rc, syscall, count;

  ret = waitpid(child_pid, &status, 0);
  if (ret == -1 || WIFSTOPPED(status) == 0)
  {
    kill_process(child_pid);
    exit(1);
  }

  rc = ptrace(PTRACE_SETOPTIONS, child_pid, NULL, PTRACE_O_TRACESYSGOOD);
  if (rc)
  {
    kill_process(child_pid);
    exit(1);
  }

  // Tracing child
  while (1)
  {
    // Wait for syscall
    ptrace(PTRACE_SYSCALL, child_pid, 0, 0);
    ret = waitpid(child_pid, &status, 0);

    if (ret == -1 || ret == 0)
    {
      puts("[Sandbox] Error waitpid");
      kill_process(child_pid);
      exit(1);
    }

    if (WIFEXITED(status))
    {
      // Normal termination
      break;
    }

    if (WIFSIGNALED(status))
    {
      puts("[Sandbox] Unhandled signal");
      break;
    }

    if (WCOREDUMP(status))
    {
      puts("[Sandbox] Runtime Error");
      break;
    }

    if (WIFSTOPPED(status))
    {
      // Catch known signal
      if (WSTOPSIG(status) <= 32)
      {
        if (WSTOPSIG(status) == 11)
        {
          puts("[Sandbox] Segmentation Fault");
        }
        else if (WSTOPSIG(status) == 4)
        {
          puts("[Sandbox] Illegal Instruction");
        }
        else if (WSTOPSIG(status) == 7)
        {
          puts("[Sandbox] Bus Error");
        }
        break;
      }

      ptrace(PTRACE_GETREGS, child_pid, 0, &regs);

      // syscall number = rax
      syscall = regs.orig_rax;

      // Don't allow EXECVE and EXECVEAT
      if (syscall == EXECVE || syscall == EXECVEAT)
      {
        puts("[Sandbox] Forbidden syscall detected. Kill the program.");
        kill_process(child_pid);
        break;
      }
    }
    else
    {
      break;
    }
  }

  exit(0);
}

int main(int argc, char** argv)
{
  init();

  pid_t child_pid;

  if (argc < 2)
  {
    fprintf(stderr, "Run: %s [program]\n", argv[0]);
    return -1;
  }

  child_pid = fork();
  if (child_pid == 0)
  {
    run_target(argv[1]);
  }
  else if (child_pid > 0)
  {
    run_debugger(child_pid);
  }
  else
  {
    fprintf(stderr, "Fork error");
    return -1;
  }

  return 0;
}
