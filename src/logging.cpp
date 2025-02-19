#include "logging.hpp"

namespace Logging {
	uint8_t SAVED_LOG_LEVEL = 0;
	uint8_t LOG_LEVEL =
	    ERROR_LVL | SUCCESS_LVL | DEBUG_LVL | WARN_LVL | INFO_LVL | LOG_WRITE;

	std::string getCurrentTime() {
	  std::time_t now = std::time(nullptr);
	  std::tm* local_time = std::localtime(&now);
	  char buffer[80];
	  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time);
	  return std::string(buffer);
	}

	void writeToFile(const char* prefix, const char* msg, va_list args) {
	  if(LOG_LEVEL & LOG_WRITE){
		  std::ofstream log_file("log.txt", std::ios_base::app);
		  if (log_file.is_open()) {
		    log_file << getCurrentTime() << " " << prefix << " ";
		    char buffer[1024];
		    vsnprintf(buffer, sizeof(buffer), msg, args);
		    log_file << buffer << std::endl;
		    log_file.close();
		  }
	  }
	}

	void disable_msg() {
	  SAVED_LOG_LEVEL = LOG_LEVEL;
	  LOG_LEVEL = 0;
	}

	void disable_file_logging() {
	  LOG_LEVEL &= ~LOG_WRITE;
	}

	void enable_msg() { 
		LOG_LEVEL = SAVED_LOG_LEVEL; 
	}

	void error(const char* msg, ...) {
	  if (LOG_LEVEL & ERROR_LVL) {
	    fprintf(stderr, "%s ", RED "[ERR] " ANSI_COLOR_RESET);
	    va_list args;
	    va_start(args, msg);
	    vprintf(msg, args);
	    printf("\n");
	    writeToFile("[ERR]", msg, args);
	    va_end(args);
	  }
	}

	void success(const char* msg, ...) {
	  if (LOG_LEVEL & SUCCESS_LVL) {
	    printf("%s", GRN "[SUCCESS] " ANSI_COLOR_RESET);
	    va_list args;
	    va_start(args, msg);
	    vprintf(msg, args);
	    printf("\n");
	    writeToFile("[SUCCESS]", msg, args);
	    va_end(args);
	  }
	}

	void info(const char* msg, ...) {
	  if (LOG_LEVEL & INFO_LVL) {
	    printf("%s", CYN "[INFO] " ANSI_COLOR_RESET);
	    va_list args;
	    va_start(args, msg);
	    vprintf(msg, args);
	    printf("\n");
	    writeToFile("[INFO]", msg, args);
	    va_end(args);
	  }
	}

	void debug(const char* msg, ...) {
	  if (LOG_LEVEL & DEBUG_LVL) {
	    va_list args;
	    va_start(args, msg);
	    writeToFile("[LOG]", msg, args);
	    va_end(args);
	  }
	}

	void write(const char* msg, ...) {
	  if (LOG_LEVEL & DEBUG_LVL) {
	    va_list args;
	    va_start(args, msg);
	    writeToFile("[LOG]", msg, args);
	    va_end(args);
	  }
	}

	void warn(const char* msg, ...) {
	  if (LOG_LEVEL & WARN_LVL) {
	    printf("%s", YEL "[WARN] " ANSI_COLOR_RESET);
	    va_list args;
	    va_start(args, msg);
	    vprintf(msg, args);
	    printf("\n");
	    writeToFile("[WARN]", msg, args);
	    va_end(args);
	  }
	}

	void critical(const char* msg, ...) {
	  if (LOG_LEVEL & CRITICAL_LVL) {
	    printf("%s", RED "[CRITICAL] " ANSI_COLOR_RESET);
	    va_list args;
	    va_start(args, msg);
	    vprintf(msg, args);
	    printf("\n");
	    writeToFile("[CRITICAL]", msg, args);
	    va_end(args);
	  }
	}
}