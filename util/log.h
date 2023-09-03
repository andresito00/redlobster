#ifndef UTIL_LOG_H_
#define UTIL_LOG_H_
#include <iostream>
#include <string>

static inline void trace_log(std::string file, int line, std::string msg,
                             std::ostream& out = std::cout)
{
  out << file << ": " << line << ": " << msg << '\n';
}

static inline std::string trace_log_str(std::string file, int line,
                                        std::string msg)
{
  return std::string(file) + ": " + std::to_string(line) + ": " +
         std::string(msg);
}

static inline void log_err(std::string msg, std::ostream& out = std::cout)
{
  out << "E " << msg << '\n';
}

#define LOG_ERROR(x) log_err(x)

#define TRACE_LOG(x) trace_log(__FILE__, __LINE__, x)
#define TRACE_LOG_STRING(x) trace_log_str(__FILE__, __LINE__, x)
#define TRACE_LOG_OSTREAM(x, y) trace_log(__FILE__, __LINE__, x, y)

#endif  // UTIL_LOG_H_
