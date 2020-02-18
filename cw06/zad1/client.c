#include "chat.h"

int client_queue;
int server_queue;
int client_id;
pid_t child_pid;

void at_exit() {
    if(child_pid != 0) {
        msgctl(client_queue, IPC_RMID, NULL);
        send_data(server_queue, STOP, client_id, 0);
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
        strtok(command, " ");
        char *saveptr;
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

int main(int argc, char **argv) {

    text = malloc(MAX_MESSAGE_LEN);
    key_t key = ftok(getenv("HOME"), 0);

    if((server_queue = msgget(key, 0)) == -1) {
        fprintf(stderr, "Error with getting server's queue\n");
        exit(1);
    }

    key_t client_key = ftok(getenv("HOME"), getpid());

    if((client_queue = msgget(client_key, IPC_EXCL | IPC_CREAT | 0666)) == -1) {
        fprintf(stderr, "Error with creating client's queue\n");
        exit(1);
    }

    atexit(at_exit);
    send_data(server_queue, INIT, client_key, 0);
    receive_data(client_queue, NULL, &client_id, NULL);

    printf("My id: %d\n", client_id);

    size_t max_comm_len = MAX_MESSAGE_LEN + 6;
    char *command = malloc(MAX_MESSAGE_LEN + 6);

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
                char *comm = strtok_r(input, "\n", &saveptr);
                while(comm != NULL) {
                    char tmp[strlen(comm) + 1];
                    int len = sprintf(tmp, "%s\n", command);
                    handle_input(tmp, len);
                    comm = strtok_r(NULL, "\n", &saveptr);
                }
            }
        }
        while(1) {
            size_t size = getline(&command, &max_comm_len, stdin);
            handle_input(command, size);
        }
    }
}