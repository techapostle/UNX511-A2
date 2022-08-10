#pragma once

#include "Logger.h"

#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX_BUFFER_SIZE 256

// Global variables
const int server_port = 4201;
struct sockaddr_in server_addr;
int sock_fd;
char buffer[MAX_BUFFER_SIZE];
LOG_LEVEL global_log_level = ERROR;
std::string server_ip = "127.0.0.1";
socklen_t socket_len;

// Thread variables
bool is_running = true;
std::mutex logger_mutex;

// Thread functions declaration
void run_receiver(int fd);

int InitializeLog() {
  if ((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("ERROR: Unable to create socket");
    // exit(EXIT_FAILURE);
    return -1;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
  server_addr.sin_port = htons(server_port);
  socket_len = sizeof(server_addr);
  std::thread receive_thread(run_receiver, sock_fd);
  receive_thread.detach();
  return 1;
}

void SetLogLevel(LOG_LEVEL level) {}

void Log(LOG_LEVEL level, const char *prog, const char *func, int line,
         const char *messaage) {}

void ExitLog() {}
