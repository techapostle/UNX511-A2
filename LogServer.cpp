// LogServer.cpp - Server for logging
//
// 12-Mar-19  M. Watler         Created.
//

#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

using namespace std;

const int PORT = 1153;
const char IP_ADDR[] = "192.168.50.107";
const char logFile[] = "logFile.txt";
const int BUF_LEN = 4096;
char buf[BUF_LEN];
bool is_running;
struct sockaddr_in remaddr;
socklen_t addrlen;

pthread_mutex_t lock_x;

void *recvThread(void *arg);

static void shutdownHandler(int sig) {
  switch (sig) {
  case SIGINT:
    is_running = false;
    break;
  case (2):
    dump_log();
    break;
  case (0):
    shutdown();
    break;
  default:
    cout << "INVALID COMMAND: Please use 1, 2 or 0" << endl << endl;
  }
}

int main(void) {
  int fd, ret, len;
  struct sockaddr_in myaddr;
  addrlen = sizeof(remaddr);

  signal(SIGINT, shutdownHandler);

  fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
  if (fd < 0) {
    cout << "Cannot create the socket" << endl;
    cout << strerror(errno) << endl;
    return -1;
  }

  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  ret = inet_pton(AF_INET, IP_ADDR, &myaddr.sin_addr);
  if (ret == 0) {
    cout << "No such address" << endl;
    cout << strerror(errno) << endl;
    close(fd);
    return -1;
  }
  myaddr.sin_port = htons(PORT);

  ret = bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr));
  if (ret < 0) {
    cout << "Cannot bind the socket to the local address" << endl;
    cout << strerror(errno) << endl;
    return -1;
  }

  // Initialize mutex protecting
  pthread_mutex_init(&lock_x, NULL);

  // Start the receive thread
  pthread_t recvId;
  ret = pthread_create(&recvId, NULL, recvThread, &fd);

  int selection = -1;
  while (selection != 0) {
    system("clear");
    cout << " 1. Set the log level" << endl;
    cout << " 2. Dump the log file here" << endl;
    cout << " 0. Shut down" << endl;
    cin >> selection;
    if (selection >= 0 && selection <= 2) {
      cout << endl;
      int level, fdIn, numRead;
      char key;
      switch (selection) {
      case 1:
        cout << "What level? (0-Debug, 1-Warning, 2-Error, 3-Critical):";
        cin >> level;
        if (level < 0 || level > 3)
          cout << "Incorrect level" << endl;
        else {
          pthread_mutex_lock(&lock_x);
          memset(buf, 0, BUF_LEN);
          len = sprintf(buf, "Set Log Level=%d", level) + 1;
          sendto(fd, buf, len, 0, (struct sockaddr *)&remaddr, addrlen);
          pthread_mutex_unlock(&lock_x);
        }
        break;
      case 2:
        fdIn = open(logFile, O_RDONLY);
        numRead = 0;
        pthread_mutex_lock(&lock_x);
        do {
          numRead = read(fdIn, buf, BUF_LEN);
          cout << buf;
        } while (numRead > 0);
        pthread_mutex_unlock(&lock_x);
        close(fdIn);
        cout << endl << "Press any key to continue: ";
        cin >> key;
        break;
      case 0:
        is_running = false;
        break;
      }
    }
  }

  pthread_join(recvId, NULL);
  close(fd);
  return 0;
}

void *recvThread(void *arg) {
  int *fd = (int *)arg;

  int openFlags = O_CREAT | O_WRONLY | O_TRUNC;
  mode_t filePerms =
      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; /* rw-rw-rw- */
  int fdOut = open(logFile, openFlags, filePerms);

  int len;
  is_running = true;
  while (is_running) {
    pthread_mutex_lock(&lock_x);
    len =
        recvfrom(*fd, buf, BUF_LEN, 0, (struct sockaddr *)&remaddr, &addrlen) -
        1;
    if (len < 0) {
      pthread_mutex_unlock(&lock_x);
      sleep(1);
    } else {
      write(fdOut, buf, len);
      pthread_mutex_unlock(&lock_x);
    }
  }

  cout << "pthread_exit" << endl;
  pthread_exit(NULL);
}
