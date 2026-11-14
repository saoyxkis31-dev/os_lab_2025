#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <signal.h>
#include "find_min_max.h"
#include "utils.h"

pid_t *child_pids;
int pnum_global = 0;
bool timeout_triggered = false; // флаг срабатывания таймаута

void kill_children(int sig) {
  timeout_triggered = true; // помечаем, что сработал таймер
  for (int i = 0; i < pnum_global; i++) {
    if (child_pids[i] > 0) {
      kill(child_pids[i], SIGKILL);
    }
  }
  printf("\ntimeout! killed child processes.\n");
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int p3 = -1;
  int timeout = -1;
  bool with_files = false;

  while (true) {
    int cur_opt = optind ? optind : 1;
    static struct option opts[]  = {
      {"seed", required_argument, 0, 0},
      {"array_size", required_argument, 0, 0},
      {"pnum", required_argument, 0, 0},
      {"by_files", no_argument, 0, 'f'},
      {"timeout", required_argument, 0, 0},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", opts, &option_index);

    if (c == -1) break;
    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) return 1;
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) return 1;
            break;
          case 2:
            p3 = atoi(optarg);
            if (p3 <= 0) return 1;
            break;
          case 3:
            with_files = true;
            break;
          case 4:
            timeout = atoi(optarg);
            if (timeout <= 0) return 1;
            break;
        }
        break;
      case 'f':
        with_files = true;
        break;
      default:
        break;
    }
  }

  if (optind < argc || seed == -1 || array_size == -1 || p3 == -1) {
    printf("Usage: %s --seed num --array_size num --pnum num [--timeout num]\n", argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int living_child = 0;

  int channel_fds[p3][2];
  if (!with_files) {
    for (int idx = 0; idx < p3; idx++) {
      if (pipe(channel_fds[idx]) < 0) return 1;
    }
  }

  child_pids = malloc(sizeof(pid_t) * p3);
  pnum_global = p3;

  if (timeout > 0) {
    signal(SIGALRM, kill_children);
    alarm(timeout);
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  int step_size = array_size / p3;

  for (int i = 0; i < p3; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      child_pids[i] = child_pid;
      living_child += 1;
      if (child_pid == 0) {
        unsigned int left_idx = i * step_size;
        unsigned int right_idx = (i == p3 - 1) ? array_size : (i + 1) * step_size;
        struct MinMax local_result = GetMinMax(array, left_idx, right_idx);

        if (with_files) {
          char output_filename[64];
          sprintf(output_filename, "task%d.tmp", i);
          FILE *fp = fopen(output_filename, "w");
          if (fp != NULL) {
            fprintf(fp, "%d %d\n", local_result.min, local_result.max);
            fclose(fp);
          }
        } else {
          close(channel_fds[i][0]);
          write(channel_fds[i][1], &local_result, sizeof(struct MinMax));
          close(channel_fds[i][1]);
        }
        free(array);
        return 0;
      }
    } else {
      return 1;
    }
  }

  while (living_child > 0) {
    int status;
    pid_t wpid = waitpid(-1, &status, WNOHANG);
    if (wpid > 0) {
      living_child -= 1;
    } else if (wpid == 0) {
      usleep(10000);
    }
  }

  // если все успели, выключаем будильник
  if (timeout > 0) {
    alarm(0);
  }

  // ЕСЛИ БЫЛ ТАЙМАУТ - ВЫХОДИМ! НЕЛЬЗЯ ЧИТАТЬ ИЗ ТРУБ МУСОР
  if (timeout_triggered) {
    free(array);
    free(child_pids);
    return 1;
  }

  // Сюда доходим ТОЛЬКО если дети выжили и всё посчитали
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;
  for (int i = 0; i < p3; i++) {
    int min = INT_MAX;
    int max = INT_MIN;
    if (with_files) {
      char input_filename[64];
      sprintf(input_filename, "task%d.tmp", i);
      FILE *p = fopen(input_filename, "r");
      if (p != NULL) {
        fscanf(p, "%d %d", &min, &max);
        fclose(p);
        remove(input_filename);
      }
    } else {
      struct MinMax received_data;
      close(channel_fds[i][1]);
      read(channel_fds[i][0], &received_data, sizeof(struct MinMax));
      close(channel_fds[i][0]);
      min = received_data.min;
      max = received_data.max;
    }
    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);
  double time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;
  free(array);
  free(child_pids);

  printf("min: %d\n", min_max.min);
  printf("max: %d\n", min_max.max);
  printf("time: %fms\n", time);
  fflush(NULL);
  return 0;
}