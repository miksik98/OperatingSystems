#include "chat.h"

void send_data(int id, int type, int client_id, int text_length) {
    Message message;
    message.mtype = type;
    message.client_id = client_id;
    message.text_size = text_length;
    if(text_length > MAX_MESSAGE_LEN) {
        fprintf(stderr, "Message too long\n");
        exit(-1);
    }
    if(text_length > 0) {
        strcpy(message.text, text);
    }
    if(msgsnd(id, &message, MSG_SIZE, 0) == -1) {
        fprintf(stderr, "Error with sending data\n");
        exit(1);
    }
}

void receive_data(int id, int *type, int *client_id, int *text_length) {
    Message message;
    if(msgrcv(id, &message, MSG_SIZE, 0, 0) == -1) {
        fprintf(stderr, "Error with receiving data\n");
        exit(1);
    }
    if(type != NULL) *type = message.mtype;
    if(client_id != NULL) *client_id = message.client_id;
    if(text_length != NULL) {
        *text_length = message.text_size;
        if((*text_length) > 0) {
            text = strcpy(text, message.text);
        }
    }
}