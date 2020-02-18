#include "chat.h"

mqd_t server_queue;

int last_client_id = 0; //actual size of clients array
int clients[MAX_CLIENTS_NUM];
int friends_num = 0; //actual size of friends array
int friends[MAX_FRIENDS_NUM];

void init_handler(int key) {
    char path[100];
    sprintf(path,"/%d",key);
    if((clients[last_client_id] = mq_open(path, O_WRONLY)) == -1) {
        fprintf(stderr, "Error with initializing client's queue\n");
        exit(1);
    }
    send_data(clients[last_client_id], INIT, last_client_id, 0);
    last_client_id++;
}

void echo_handler(int client_id, int text_length) {
    time_t now;
    time(&now);
    char * tmp = text;
    size_t length = sprintf(text, "%.*s%s", text_length, tmp, ctime(&now));
    send_data(clients[client_id], ECHO, 0, length);
}

void to_one_handler(int client_id, int text_length) {
    if(clients[client_id] != 0) {
        time_t now;
        time(&now);
        char * tmp = text;
        size_t length = sprintf(text, "%.*s%sFrom id: %d\n",text_length, tmp, ctime(&now), client_id);
        send_data(clients[client_id], ECHO, 0, length);
    }
}

void list_handler() {
    printf("Clients' list:\n");
    for(int i = 0; i < last_client_id; i++) {
        if(clients[i] != 0) {
            printf("  id: %d, queue_id: %d\n", i, clients[i]);
        }
    }
}

void to_all_handler(int client_id, int text_length) {
    time_t now;
    time(&now);
    char * tmp = text;
    size_t length = sprintf(text, "%.*s%sFrom id: %d\n",text_length, tmp, ctime(&now), client_id);
    for(int i = 0; i < last_client_id; i++) {
        if(clients[i] != 0) {
            send_data(clients[i], ECHO, 0, length);
        }
    }
}

void friends_handler(int text_length) {
    char *tmp = malloc(text_length);
    strcpy(tmp, text);
    char *str = strtok(tmp, " ");
    friends_num = 0;
    while(str != NULL) {
        int exists = 0;
        int id = atoi(str);
        for(int i = 0; i < friends_num; i++) {
            if(friends[i] == id) {
                exists = 1;
                break;
            }
        }
        if(!exists) {
            friends[friends_num++] = id;
        }
        else{
            printf("Client with id %d is already a friend\n", id);
        }
        str = strtok(NULL, " ");
    }
}

void add_friends_handler(int text_length) {
    char *tmp = malloc(text_length);
    strcpy(tmp, text);
    char *str = strtok(tmp, " ");
    while(str != NULL) {
        int exists = 0;
        int id = atoi(str);
        for (int i = 0; i < friends_num; i++) {
            if (friends[i] == id) {
                exists = 1;
            }
        }
        if (!exists) {
            friends[friends_num++] = id;
        } else {
            printf("Client with id %d already exists!\n", id);
        }
        str = strtok(NULL, " ");
    }
}

void del_friends_handler(int text_length) {
    char *tmp = malloc(text_length);
    strcpy(tmp, text);
    char *str = strtok(tmp, " ");
    while(str != NULL) {
        int id = atoi(str);
        for (int i = 0; i < friends_num; i++) {
            if (friends[i] == id) {
                for (int j = i; j < friends_num - 1; j++) {
                    friends[j] = friends[j + 1];
                }
                friends_num--;
            }
        }
        str = strtok(NULL, " ");
    }
}

void to_friends_handler(int client_id, int text_length) {
    time_t now;
    time(&now);
    char * tmp = text;
    size_t length = sprintf(text, "%.*s%sFrom id: %d\n",text_length, tmp, ctime(&now), client_id);
    for(int i = 0; i < friends_num; i++) {
        if(clients[friends[i]] != 0) {
            send_data(clients[friends[i]], ECHO, 0, length);
        }
    }
}

void stop_handler(int client_id) {
    clients[client_id] = 0;
    mq_close(clients[client_id]);
    int active_client_exists = 0;
    for(int i = 0; i < last_client_id; i++) {
        if(clients[i] != 0) {
            active_client_exists = 1;
            break;
        }
    }
    if(!active_client_exists) {
        printf("No active clients\n");
        exit(0);
    }
}

void received_handler(int type, int client_id, int text_length) {
    switch(type) {
        case INIT:
            init_handler(client_id);
            break;
        case ECHO:
            echo_handler(client_id, text_length);
            break;
        case TO_ONE:
            to_one_handler(client_id, text_length);
            break;
        case LIST:
            list_handler();
            break;
        case TO_ALL:
            to_all_handler(client_id, text_length);
            break;
        case FRIENDS:
            friends_handler(text_length);
            break;
        case ADD_FRIENDS:
            add_friends_handler(text_length);
            break;
        case DEL_FRIENDS:
            del_friends_handler(text_length);
            break;
        case TO_FRIENDS:
            to_friends_handler(client_id, text_length);
            break;
        case STOP:
            stop_handler(client_id);
            break;
    }
}

void int_handler(int signum) {
    printf("\nSIGINT signal\n");
    exit(0);
}

void at_exit() {
    for(int i = 0; i < last_client_id; i++) {
        if(clients[i] != 0) {
            send_data(clients[i], STOP, 0, 0);
            mq_close(clients[i]);
            receive_data(server_queue, NULL, NULL, NULL);
        }
    }
    mq_close(server_queue);
    mq_unlink(SERVER);
}

int main() {
    text = malloc(MAX_MESSAGE_LEN);
    signal(SIGINT, int_handler);
    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MESSAGE_NUM;
    attr.mq_msgsize = MSG_SIZE;
    if((server_queue = mq_open(SERVER, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr)) == -1) {
        fprintf(stderr, "Error with creating server's queue\n");
        exit(1);
    }
    atexit(at_exit);
    int type, client_id, text_length;
    while(1) {
        receive_data(server_queue, &type, &client_id, &text_length);
        received_handler(type, client_id, text_length);
    }
}

