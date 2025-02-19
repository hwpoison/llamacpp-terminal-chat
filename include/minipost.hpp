#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>

#ifdef __WIN32__
    #include <winsock2.h>
    #include <windows.h>
    typedef SOCKET SocketType;
    #define Socket_error SOCKET_ERROR
    #define Invalid_socket INVALID_SOCKET
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    typedef int32_t SocketType;
    #define Socket_error -1
    #define Invalid_socket -1
#endif

#include "logging.hpp"
#include "terminal.hpp"

#define BUFFER_SIZE 10000 // 10kb
#define HTTP_INTERNAL_ERROR 500
#define HTTP_OK 200

// JSON Type
typedef std::string json;

struct Response {
  int16_t Status = -1;
  std::string body;
};

struct CallbackBus {
  virtual ~CallbackBus() = default;
  std::string buffer;
  bool stream;
};

class httpRequest {
public:
  httpRequest();
  // post http request by callback for data stream
  Response post(const char* ipaddr, 
                const int16_t port, 
                const char* endpoint,
                json payload,
                const std::function<bool(std::string chunck, const CallbackBus *bus)> &reader_callback, 
                const CallbackBus *bus);  

  SocketType  connectTo(const char* ipaddr, int16_t port);
  bool        sendData(SocketType connection, const char* data);
  bool        closeConnection(SocketType connection);
  int         getLastError();
  void        cleanUp();

  ~httpRequest();
};
