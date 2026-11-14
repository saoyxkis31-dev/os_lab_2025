#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "pthread.h"
#include "modulo.h"

// структура аргументов: с какого по какое число считать, модуль и поле под результат
struct FactorialArgs {
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
  uint64_t result;
};

// вычисление произведения чисел от begin до end
uint64_t Factorial(const struct FactorialArgs *args) {
  uint64_t ans = 1;
  struct FactorialArgs val = *args;

  // перемножаем числа и берём остаток через MultModulo
  for (uint64_t i = val.begin; i <= val.end; i++) {
    ans = MultModulo(ans, i, val.mod);
  }

  return ans;
}

// функция потока: считает факториал и кладет в поле result
void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  struct FactorialArgs val = *fargs;
  val.result = Factorial(fargs);
  *fargs = val; // сохраняем результат обратно в массив args
  return NULL;
}

int main(int argc, char **argv) {
  int tnum = -1;
  int port = -1;

  // аргументы командной строки --port и --tnum
  while (true) {
    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"tnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        port = atoi(optarg);
        break;
      case 1:
        tnum = atoi(optarg);
        break;
      default:
        printf("индекс %d вне опций\n", option_index);
      }
    } break;

    case '?':
      printf("какая-то лишняя шляпа\n");
      break;
    default:
      fprintf(stderr, "код возврата: %o\n", c);
    }
  }

  if (port == -1 || tnum == -1) {
    fprintf(stderr, "надо так: %s --port 20001 --tnum 4\n", argv[0]);
    return 1;
  }

  // создание сетевого tcp сокета
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    fprintf(stderr, "не удалось создать сокет сервера\n");
    return 1;
  }

  // настройки адреса и порта
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port);
  server.sin_addr.s_addr = htonl(INADDR_ANY); // слушаем все сетевые интерфейсы

  // разрешаем сразу переиспользовать порт без ожидания
  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  // привязка сокета к порту
  int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
  if (err < 0) {
    fprintf(stderr, "нельзя привязать сокет к порту\n");
    return 1;
  }

  // перевод сокета в режим прослушивания
  err = listen(server_fd, 128);
  if (err < 0) {
    fprintf(stderr, "не можем слушать сокет\n");
    return 1;
  }
  printf("слушаем сервер на порту %d\n", port);

  // сервер ждёт подключений клиентов
  while (true) {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);

    // ждем подключения от клиента
    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

    if (client_fd < 0) {
      fprintf(stderr, "не можем установить соединение с клиентом\n");
      continue;
    }

    while (true) {
      // буфер под begin end mod
      unsigned int buffer_size = sizeof(uint64_t) * 3;
      char from_client[buffer_size];
      int read = recv(client_fd, from_client, buffer_size, 0);

      if (!read)
        break; // клиент отключился
      if (read < 0) {
        fprintf(stderr, "ошибка чтения от клиента\n");
        break;
      }
      if (read < buffer_size) {
        fprintf(stderr, "клиент отправил кривой формат данных\n");
        break;
      }

      // извлекаем переданные числа из буфера
      uint64_t begin = 0;
      uint64_t end = 0;
      uint64_t mod = 0;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));
      fprintf(stdout, "получено: %lu %lu %lu\n", begin, end, mod);

      // массивы для управления потоками
      pthread_t threads[tnum];
      struct FactorialArgs args[tnum];

      // считаем общий размер диапазона и шаг на один поток
      uint64_t total_numbers = end - begin + 1;
      uint64_t step = total_numbers / tnum;

      // задачи потокам
      for (int i = 0; i < tnum; i++) {
        args[i].begin = begin + i * step;
        args[i].mod = mod;
        args[i].result = 1;

        if (i == tnum - 1) {
          args[i].end = end;
        } else {
          args[i].end = args[i].begin + step - 1;
        }

        // запуск потока
        if (pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i])) {
          fprintf(stderr, "не можем создать поток\n");
          return 1;
        }
      }

      // дожидаемся результатов и перемножаем их
      uint64_t total = 1;
      for (int i = 0; i < tnum; i++) {
        pthread_join(threads[i], NULL);
        total = MultModulo(total, args[i].result, mod);
      }

      printf("всего: %lu\n", total);

      // отправляем посчитанный результат обратно клиенту
      char buffer[sizeof(total)];
      memcpy(buffer, &total, sizeof(total));
      err = send(client_fd, buffer, sizeof(total), 0);
      if (err < 0) {
        fprintf(stderr, "не можем отправить данные клиенту\n");
        break;
      }
    }

    // закрываем сокет клиента после окончания обмена
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
  }

  return 0;
}