#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int builtin_cd(char **argv, int argc);
static int builtin_exit(char **argv, int argc);
static int builtin_help(char **argv, int argc);

#define N 12

extern char **environ;

char *allowed[N] = {"cp",   "touch", "mkdir", "ls", "pwd",  "cat",
                    "grep", "chmod", "diff",  "cd", "exit", "help"};

// An array of builtins corresponding to commands
int (*builtins[N])(char **, int) = {
    NULL, NULL, NULL, NULL,       NULL,         NULL,
    NULL, NULL, NULL, builtin_cd, builtin_exit, builtin_help};

int isAllowed(const char *cmd) {
  for (int i = 0; i < N; ++i) {
    if (!strcmp(cmd, allowed[i])) {
      return i;
    }
  }
  return -1;
}

int main() {

  char *arguments[21] = {NULL};

  char line[256];

  while (1) {

    // State for strtok_r because I can
    char *save_ptr = NULL;

    fprintf(stderr, "rsh>");

    if (fgets(line, 256, stdin) == NULL)
      continue;

    if (strcmp(line, "\n") == 0)
      continue;

    line[strlen(line) - 1] = '\0';

    int argc = 0;
    arguments[argc] = strtok_r(line, " ", &save_ptr);
    while ((arguments[++argc] = strtok_r(NULL, " ", &save_ptr)) != NULL &&
           argc < 20)
      ;

    arguments[20] = NULL;

    int command_idx;
    if ((command_idx = isAllowed(arguments[0])) == -1) {
      puts("NOT ALLOWED!");
      continue;
    } else if (command_idx < 9) {
      // Spawn process and wait for it to complete
      pid_t pid;
      posix_spawnattr_t attr;
      posix_spawnattr_init(&attr);

      // spawn the process
      posix_spawnp(&pid, arguments[0], NULL, &attr, arguments, environ);

      // wait for it to finish
      int status;
      waitpid(pid, &status, 0);

      posix_spawnattr_destroy(&attr);
      continue;
    } else if (command_idx < N) {
      builtins[command_idx](arguments, argc);
      continue;
    } else {
      puts("Unreachable!?");
      exit(-1);
    }

    // TODO
    // Add code to spawn processes for the first 9 commands
    // And add code to execute cd, exit, help commands
    // Use the example provided in myspawn.c
  }
  return 0;
}

int builtin_cd(char **argv, int argc) {

  if (argc != 2) {
    puts("-rsh: cd: too many arguments");
    return -1;
  }

  chdir(argv[1]);

  return 0;
}

int builtin_exit(char **argv, int argc) { exit(0); }

int builtin_help(char **argv, int argc) {
  puts("The allowed commands are:");
  for (int i = 0; i < N; i++) {
    printf("%d: %s\n", i + 1, allowed[i]);
  }
  return 0;
}
