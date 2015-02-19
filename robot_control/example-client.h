#include <string>
#ifndef EXAMPLE_CLIENT_H
#define EXAMPLE_CLIENT_H
 
// This is the content of the .h file, which is where the declarations go
void publish(std::string command);
void handle_message(const std::string & message);
void *listen(void *threadid);
void openWS();
void deleteWS();

extern bool response_received;
// This is the end of the header guard
#endif
