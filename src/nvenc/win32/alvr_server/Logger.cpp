#include "Logger.h"

#include <cstdarg>

#include "bindings.h"

void _log(const char *format, va_list args, void (*logFn)(const char *),
          bool driverLog = false) {
  char buf[1024];
  int count = vsnprintf(buf, sizeof(buf), format, args);
  if (count > (int)sizeof(buf))
    count = (int)sizeof(buf);
  if (count > 0 && buf[count - 1] == '\n')
    buf[count - 1] = '\0';

  logFn(buf);
}

Exception MakeException(const char *format, ...) {
  va_list args;
  va_start(args, format);
  Exception e = FormatExceptionV(format, args);
  va_end(args);

  return e;
}

void Error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

void Warn(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

void Info(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

void Debug(const char *format, ...) {
#ifdef ALVR_DEBUG_LOG
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
#else
  (void)format;
#endif
}

void LogPeriod(const char *tag, const char *format, ...) {
  va_list args;
  va_start(args, format);

  char buf[1024];
  int count = vsnprintf(buf, sizeof(buf), format, args);
  if (count > (int)sizeof(buf))
    count = (int)sizeof(buf);
  if (count > 0 && buf[count - 1] == '\n')
    buf[count - 1] = '\0';
  printf("[%s] %s\n", tag, buf);

  va_end(args);
}