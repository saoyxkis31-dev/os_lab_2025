#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include "modulo.h"

// структура одного сервера: ip и порт
struct Server {
  char ip[255];
  int port;
};

// структура задачи для потокаЁ
struct ClientThreadArgs {
  struct Server server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
  uint64_t result;
};

// перевод строки в 64-битное число
bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "число вышло за границы uint64_t: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

// функция потока: подключается к своему серверу и забирает посчитанный кусок
void *ThreadClient(void *args) {
  struct ClientThreadArgs *targs = (struct ClientThreadArgs *)args;
  struct ClientThreadArgs val = *targs;

  // ищем адрес сервера по доменному имени или ip
  struct hostent *hostname = gethostbyname(val.server.ip);
  if (hostname == NULL) {
    fprintf(stderr, "не удалось найти хост: %s\n", val.server.ip);
    val.result = 1;
    *targs = val;
    return NULL;
  }

  // заполняем сетевой адрес сервера
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(val.server.port);
  server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

  // создаем tcp сокет
  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) {
    fprintf(stderr, "не удалось создать сокет\n");
    val.result = 1;
    *targs = val;
    return NULL;
  }

  // подключаемся к удаленному серверу
  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "ошибка подключения к серверу %s:%d\n", val.server.ip, val.server.port);
    close(sck);
    val.result = 1;
    *targs = val;
    return NULL;
  }

  // упаковываем три числа в буфер для отправки
  char task[sizeof(uint64_t) * 3];
  memcpy(task, &val.begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &val.end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &val.mod, sizeof(uint64_t));

  // отправляем диапазон вычислений на сервер
  if (send(sck, task, sizeof(task), 0) < 0) {
    fprintf(stderr, "ошибка отправки данных\n");
    close(sck);
    val.result = 1;
    *targs = val;
    return NULL;
  }

  // принимаем посчитанное число от сервера
  char response[sizeof(uint64_t)];
  if (recv(sck, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "ошибка приема данных от сервера\n");
    close(sck);
    val.result = 1;
    *targs = val;
    return NULL;
  }

  // достаем число из байтового ответа и сохраняем
  uint64_t answer = 1;
  memcpy(&answer, response, sizeof(uint64_t));
  val.result = answer;

  // сохраняем результат обратно в аргументы и закрываем сокет
  *targs = val;
  close(sck);
  return NULL;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'};

  // чтение флагов командной строки
  while (true) {
    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        break;
      case 2:
        memcpy(servers, optarg, strlen(optarg));
        break;
      default:
        printf("индекс %d вне опций\n", option_index);
      }
    } break;

    case '?':
      printf("ошибка в аргументах\n");
      break;
    default:
      fprintf(stderr, "код возврата: %o\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "надо так: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
    return 1;
  }

  // открываем файл со списком серверов
  FILE *file = fopen(servers, "r");
  if (file == NULL) {
    fprintf(stderr, "не удалось открыть файл серверов: %s\n", servers);
    return 1;
  }

  // массив под адреса серверов
  struct Server to[64];
  unsigned int servers_num = 0;
  char line[255];

  // читаем файл построчно формата ip:port
  while (fgets(line, sizeof(line), file) != NULL) {
    char *separator = strchr(line, ':');
    if (separator != NULL) {
      *separator = '\0';
      strncpy(to[servers_num].ip, line, sizeof(to[servers_num].ip) - 1);
      to[servers_num].port = atoi(separator + 1);
      servers_num++;
    }
  }
  fclose(file);

  if (servers_num == 0) {
    fprintf(stderr, "в файле не найдено ни одного адреса сервера\n");
    return 1;
  }

  // массивы под потоки и их аргументы
  pthread_t threads[servers_num];
  struct ClientThreadArgs thread_args[servers_num];

  // распределяем диапазон [1, k] между серверами
  uint64_t step = k / servers_num;
  uint64_t current_start = 1;

  for (unsigned int i = 0; i < servers_num; i++) {
    thread_args[i].server = to[i];
    thread_args[i].begin = current_start;
    thread_args[i].mod = mod;
    thread_args[i].result = 1;

    // последний сервер забирает остаток до самого k
    if (i == servers_num - 1) {
      thread_args[i].end = k;
    } else {
      thread_args[i].end = current_start + step - 1;
    }

    current_start = thread_args[i].end + 1;

    // запускаем сетевой поток
    if (pthread_create(&threads[i], NULL, ThreadClient, (void *)&thread_args[i])) {
      fprintf(stderr, "не удалось создать поток для сервера %s:%d\n", to[i].ip, to[i].port);
      return 1;
    }
  }

  // собираем результаты со всех серверов
  uint64_t total = 1;
  for (unsigned int i = 0; i < servers_num; i++) {
    pthread_join(threads[i], NULL);
    total = MultModulo(total, thread_args[i].result, mod);
  }

  printf("итоговый ответ: %lu\n", total);
  return 0;
}