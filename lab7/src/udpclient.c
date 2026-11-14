#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// убрали #define serv_port и bufsize
#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char **argv) {
  if (argc != 4) {
    printf("нада: %s <ip> <port> <bufsize>\n", argv[0]);
    exit(1);
  }

  // порт и буферв в числа
  int serv_port = atoi(argv[2]);
  int bufsize = atoi(argv[3]);

  int sockfd, n;
  //массивы под переданный размер буфера
  char sendline[bufsize], recvline[bufsize + 1];
  struct sockaddr_in servaddr;

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  // порт из аргумента
  servaddr.sin_port = htons(serv_port);

  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 0) {
    perror("inet_pton problem");
    exit(1);
  }
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem");
    exit(1);
  }

  write(1, "Enter string\n", 13);

  // читаем и отправляем кусками по bufsize
  while ((n = read(0, sendline, bufsize)) > 0) {
    if (sendto(sockfd, sendline, n, 0, (SADDR *)&servaddr, SLEN) == -1) {
      perror("sendto problem");
      exit(1);
    }

    if (recvfrom(sockfd, recvline, bufsize, 0, NULL, NULL) == -1) {
      perror("recvfrom problem");
      exit(1);
    }
    // чтобы не печатать старый мусор
    recvline[n] = '\0';

    printf("REPLY FROM SERVER= %s\n", recvline);
  }
  close(sockfd);
}