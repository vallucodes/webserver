#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <signal.h>
#include <unistd.h>


#define TIME_OUT_POLL		100
#define TIME_OUT_REQUEST	60000
#define TIME_OUT_RESPONSE	60000
#define MAX_CLIENTS			900
#define MAX_BUFFER_SIZE		10000000
#define MAX_BODY_SIZE		10000000

#define CLIENT_DISCONNECT			" disconnected.\n"
#define CLIENT_TIMEOUT				" dropped by the server: Timeout.\n"
#define CLIENT_MALFORMED_REQUEST	" dropped by the server: Malformed request.\n"
#define CLIENT_SEND_ERROR			" dropped by the server: send() failed.\n"
