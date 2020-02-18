#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#define ECHO 1
#define LIST 2
#define FRIENDS 3
#define ADD_FRIENDS 4
#define DEL_FRIENDS 5
#define TO_ALL 6
#define TO_FRIENDS 7
#define TO_ONE 8
#define STOP 9
#define INIT 10

#define MSG_SIZE sizeof(Message) - sizeof(long)
#define MAX_MESSAGE_LEN 500
#define MAX_FRIENDS_NUM 10
#define MAX_CLIENTS_NUM 30

char *text;

typedef struct Message {
    long mtype;
    int client_id;
    size_t text_size;
    char text[MAX_MESSAGE_LEN];
} Message;

void send_data(int id, int type, int client_id, int text_length);
void receive_data(int id, int *type, int *client_id, int *text_length);
