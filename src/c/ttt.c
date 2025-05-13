#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <stdbool.h>

#include <string.h>

void printBoard(const char* board) {
    if (strlen(board) < 9) return;
    if (strlen(board) > 9) return;
    
    printf("\n-------------\n|");
    
    printf(" %c   %c   %c |\n|           |\n|           |\n| %c   %c   %c |\n|           |\n|           |\n| %c   %c   %c |\n-------------\n", board[0], board[1], board[2], board[3], board[4], board[5], board[6], board[7], board[8]);
}

void convertBoard(char* board) {
    int len = strlen(board);
    char wChar;

    for (int i = 0; i < len; ++i)
    {
        wChar = board[i];

        switch (wChar)
        {
            case '0':
            {
                wChar = '-';
                break;
            }
            case '1':
            {
                wChar = 'X';
                break;
            }
            case '2':
            {
                wChar = 'O';
                break;
            }
        }

        board[i] = wChar;
    }
}

void onCallback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) {

}

void onMessage(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	/* This blindly prints the payload, but the payload can be anything so take care. */
    char * payload = (char *)msg->payload;
    char * topic = msg->topic;

    //For results of Game
    if (strcmp(topic, "tttGame") == 0)
    {
        char * message = malloc(sizeof(char *) * strlen(payload));

        if (message == NULL)
        {
            printf("NOT ENOUGH MEMORY TO ALLOCATE!");
            return;
        }
        
        //Copy the payload into a new var
        strcpy(message, payload);
        
        //Convert the board for display
        convertBoard(message);

        //Display the board
        printBoard(message);
    }

	//printf("%s %d %s\n", msg->topic, msg->qos, (char *)msg->payload);

}

int main() {
    struct mosquitto *mosq;
    int res = mosquitto_lib_init();

    char c;
    float f;

    bool check;

    if (res == MOSQ_ERR_SUCCESS) printf("Success!\n");
    else printf("Failed...\n");

    mosq = mosquitto_new(NULL, true, NULL);

    //Function returns NULL if there is no memory
    if (mosq == NULL)
    {
        fprintf(stderr, "Error: Out of memory.\n");
        return EXIT_FAILURE;
    }

    mosquitto_message_callback_set(mosq, onMessage);

    res = mosquitto_connect(mosq, "104.196.229.154", 1883, 60);

    if (res != MOSQ_ERR_SUCCESS)
    {
        mosquitto_destroy(mosq);

        fprintf(stderr, "There was an error with connecting! %s\n", mosquitto_strerror(res));

        return EXIT_FAILURE;
    }

    res = mosquitto_loop_start(mosq);

    if (res != MOSQ_ERR_SUCCESS)
    {
        mosquitto_destroy(mosq);

        fprintf(stderr, "There was an error with the loop starting! %s\n", mosquitto_strerror(res));

        return EXIT_FAILURE;
    }

    char start;
    char mode;

    int n = 1;

    mosquitto_subscribe(mosq, NULL, "tttGame", 1);

    printf("CPP - CS 2600 - Tic Tac Toe! MQTT STYLE! Version 0.0.0\n");
    printf("Who will start? (You are x) (x or o): ");
    fgets(&start, n, stdin);

    printf("Which mode do you want to do? (1 or 2 players): ");
    fgets(&mode, n, stdin);

    while (true)
    {
        n += 1000;
        //printf("%d\n", n);
    }

    return EXIT_SUCCESS;
}
