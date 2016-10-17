// log.h
// Joshua Krueger
// 2016_10_16
// 

#ifndef LOG_H_
  #define LOG_H_
  
  // Public Prototypes
  void Message (const char *fmt, ...);
  void Error (const char *fmt, ...);
  void Log(const char *fmt, ...);
  void FLog(const char *fmt, ...);
  
  char *GetLastMessage(void);
  
#endif /* LOG_H_ */
 
