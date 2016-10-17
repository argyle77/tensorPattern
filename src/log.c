// User communications / logging
// Joshua KRueger
// 2016_10_16
//

// Defines - Some text colors
//~ #define KRED "\x1B[31m"
#define KRED "\033[1;31m"
#define KBLUE "\033[1;34m"
#define KGREEN "\033[1;32m"
#define KNRM "\033[0m"

#define TEXT_SIZE 100

// Includes
#include <stdarg.h>
#include <stdio.h>

// Globals
char lastText[TEXT_SIZE] = "";

void Log(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char tempText[TEXT_SIZE];
  
  vsnprintf(tempText, sizeof(tempText), fmt, args);
  fprintf(stdout, "%s%s%s\n", KGREEN, tempText, KNRM);

  va_end(args);
}

void FLog(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char tempText[TEXT_SIZE];
  
  vsnprintf(tempText, sizeof(tempText), fmt, args);
  fprintf(stdout, "%s%s%s", KGREEN, tempText, KNRM);

  va_end(args);
}

// Takes a printf like set of arguments and passes the message onto the user
// log / display.
void Message (const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  
  vsnprintf(lastText, sizeof(lastText), fmt, args);
  fprintf(stdout, "%s%s%s\n", KBLUE, lastText, KNRM);

  va_end(args);
}

// Takes a printf like set of arguments and passes the message onto the error
// log / display.
void Error (const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vsnprintf(lastText, sizeof(lastText), fmt, args);
  fprintf(stderr, "%sERROR:%s %s\n", KRED, KNRM, lastText);
  
  va_end(args);
}

char *GetLastMessage(void) {
  return (lastText);
}
