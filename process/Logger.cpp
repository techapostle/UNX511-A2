#include "Logger.h"

#include <iostream>
#include <mutex>
#include <thread>
#include <string>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

// Global
struct sockaddr_in server_addr;
const int server_port = 1155;
const int BUF_LEN = 4096;
int sock_fd;
char buffer[BUF_LEN];
LOG_LEVEL global_log_level = ERROR;
std::string server_ip = "127.0.0.1";
socklen_t socket_len;
int len;
bool is_running = true;
std::mutex logger_mutex;

// Thread functions declaration
void run_receiver(int fd);

// Logger.h function definitions
int InitializeLog() {
  if ((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("ERROR: Unable to create socket");
    exit(EXIT_FAILURE);
    // return -1;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
  server_addr.sin_port = htons(server_port);
  socket_len = sizeof(server_addr);
  std::thread receive_thread(run_receiver, sock_fd);
  receive_thread.detach();
  return 1;
}

void SetLogLevel(LOG_LEVEL level) { global_log_level = level; }

void Log(LOG_LEVEL level, const char *prog, const char *func, int line,
         const char *message) {
  if (level > global_log_level) {
    time_t now = time(0);
    char *dt = ctime(&now);
    memset(buffer, 0, BUF_LEN);
    char levelStr[][16] = {"DEBUG", "WARNING", "ERROR", "CRITICAL"};
    len = sprintf(buffer, "%s %s %s:%s:%d %s\n", dt, levelStr[level], prog,
                  func, line, message) + 1;
    buffer[len - 1] = '\0';

    sendto(sock_fd, buffer, len, 0, (struct sockaddr *)&server_addr,
           socket_len);
  }
}

void ExitLog() {
  is_running = false;
  close(sock_fd);
}


// Thread receiver function
void run_receiver(int fd) {
  // Set 1 second timeout for the socket
  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = 1;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));

  // Run receiver loop
  while (is_running) {
    memset(buffer, 0, BUF_LEN);
    logger_mutex.lock();
    int size = recvfrom(fd, buffer, BUF_LEN, 0, (struct sockaddr *)&server_addr,
                        &socket_len);
    std::string message = buffer;

    if (size > 0) {
      std::string log_level = message.substr((message.find("=") + 1));

      if (log_level == "DEBUG") {
        global_log_level = DEBUG;
      } else if (log_level == "WARNING") {
        global_log_level = WARNING;
      } else if (log_level == "ERROR") {
        global_log_level = ERROR;
      } else if (log_level == "CRITICAL") {
        global_log_level = CRITICAL;
      } else {
        continue;
      }
    } else {
      sleep(1);
    }
    message = "";
    logger_mutex.unlock();
  }
}
