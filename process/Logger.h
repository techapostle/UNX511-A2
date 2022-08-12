#ifndef LOGGER_H
#define LOGGER_H

#include <string>

enum LOG_LEVEL { DEBUG = -1, WARNING = 0, ERROR = 1, CRITICAL = 2 };

int InitializeLog();

void SetLogLevel(LOG_LEVEL level);

void Log(LOG_LEVEL level, const char *prog, const char *func, int line,
         const char *messaage);

void ExitLog();

#endif
