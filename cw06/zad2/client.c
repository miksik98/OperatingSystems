#include "chat.h"

int client_queue;
int server_queue;
int client_id;
pid_t child_pid;

void send_echo(char *string, size_t size) {
    strcpy(text, string + 5);
    send_data(server_queue, ECHO, client_id, size);
}

void send_list() {
    send_data(server_queue, LIST, 0, 0);
}

void send_to_all(char *string, size_t size) {
    strcpy(text, string + 5);
    send_data(server_queue, TO_ALL, client_id, size);
}

void send_to_one(char *string, int id, size_t size) {
    strcpy(text, string);
    send_data(server_queue, TO_ONE, id, size);
}

void send_friends(char *string, size_t size) {
    strcpy(text, string + 8);
    send_data(server_queue, FRIENDS, client_id, size);
}

void send_add_friends(char *string, size_t size) {
    strcpy(text, string + 4);
    send_data(server_queue, ADD_FRIENDS, client_id, size);
}

void send_del_friends(char *string, size_t size) {
    strcpy(text, string + 4);
    send_data(server_queue, DEL_FRIENDS, client_id, size);
}

void send_to_friends(char *string, size_t size) {
    strcpy(text, string + 9);
    send_data(server_queue, TO_FRIENDS, client_id, size);
}

void handle_input(char *command, size_t size) {
    if(strncmp(command, "ECHO", 4) == 0) {
        send_echo(command, size - 5);
    }
    else if(strcmp(command, "LIST\n") == 0) {
        send_list();
    }
    else if(strncmp(command, "2ALL", 4) == 0) {
        send_to_all(command, size - 5);
    }
    else if(strncmp(command, "2ONE", 4) == 0) {
        char * saveptr;
        strtok_r(command, " ", &saveptr);
        char *str = strtok_r(command + 5, " ", &saveptr);
        if(str == NULL) {
            fprintf(stderr, "Wrong number of arguments\n");
            return;
        }
        int id = atoi(str);
        str = strtok_r(NULL, "", &saveptr);
        size_t size = 0;
        if(str != NULL) size = strlen(str);
        send_to_one(str, id, size);
    }
    else if(strncmp(command, "FRIENDS", 7) == 0) {
        send_friends(command, size - 8);
    }
    else if(strncmp(command, "ADD", 3) == 0) {
        send_add_friends(command, size - 4);
    }
    else if(strncmp(command, "DEL", 3) == 0) {
        send_del_friends(command, size - 4);
    }
    else if(strncmp(command, "2FRIENDS", 8) == 0) {
        send_to_friends(command, size - 9);
    }
    else if(strcmp(command, "STOP\n") == 0) {
        exit(0);
    }
    else{
        printf("Wrong command\n");
    }
}

void at_exit() {
    if(child_pid != 0) {
        mq_close(client_queue);
        char path[100];
        sprintf(path, "/%d", getpid());
        mq_unlink(path);
        send_data(server_queue, STOP, client_id, 0);
        mq_close(server_queue);
        kill(child_pid, SIGKILL);
    }
}

void sigint_handler(int signum) {
    printf("\nSIGINT signal\n");
    exit(0);
}

void sigusr_handler(int signum) {
    printf("Server stopped\n");
    exit(0);
}

int main(int argc, char **argv) {

    text = malloc(MAX_MESSAGE_LEN);
    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MESSAGE_NUM;
    attr.mq_msgsize = MSG_SIZE;
    char path[100];
    sprintf(path, "/%d", getpid());
    if((client_queue = mq_open(path, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr)) == -1){
        fprintf(stderr, "Error with creating clients' queue\n");
        exit(1);
    }
    if((server_queue = mq_open(SERVER, O_WRONLY)) == -1) {
        fprintf(stderr, "Error with opening server's queue\n");
        exit(1);
    }
    atexit(at_exit);
    send_data(server_queue, INIT, getpid(), 0);
    receive_data(client_queue, NULL, &client_id, NULL);
    printf("My id: %d\n", client_id);
    size_t max_comm_length = MAX_MESSAGE_LEN + 8;
    char *command = malloc(MAX_MESSAGE_LEN + 8);

    if((child_pid = fork()) == 0) {
        int type, client_id, text_length;
        while(1) {
            receive_data(client_queue, &type, &client_id, &text_length);
            if(type == ECHO) {
                printf("%.*s\n", text_length, text);
            }
            if(type == STOP) {
                kill(getppid(), SIGUSR1);
                exit(0);
            }
        }
    }
    else {
        signal(SIGINT, sigint_handler);
        signal(SIGUSR1, sigusr_handler);
        if(argc > 1) {
            FILE *fd;
            if((fd = fopen(argv[1], "r")) != NULL) {
                char input[10000];
                fread(input, sizeof(char), 10000, fd);
                char *saveptr;
                char *command = strtok_r(input, "\n", &saveptr);
                while(command != NULL) {
                    char tmp[strlen(command) + 1];
                    int len = sprintf(tmp, "%s\n", command);
                    handle_input(tmp, len);
                    command = strtok_r(NULL, "\n", &saveptr);
                }
            }
        }
        while(1) {
            size_t size = getline(&command, &max_comm_length, stdin);
            handle_input(command, size);
        }
    }
}