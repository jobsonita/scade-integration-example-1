#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


#include "ua.h"
#include "project1.h"

#include "interfaces.h"

#ifndef TRUE
#define TRUE kcg_true
#define FALSE kcg_false
#endif

/* the port users will be connecting to */
#define DBUSPORT 4950

#define MSG_WAIT_OPTION 0

extern int gethostname(char *, size_t);
extern int clock_gettime(clockid_t, struct timespec*);

char *DF_server_ip = "127.0.0.1";
int DF_server_port = 1231;
int DF_server_socket;

char *to_outside_ip = "127.0.0.1";
int to_outside_socket;
struct sockaddr_in to_outside_address;

int from_outside_socket;


inC_project1 ua_inputs;
outC_project1 ua_outputs;

pthread_t thread_receiver;
pthread_mutex_t lock;


void connectToDFServer() {
    printf("DF server IP...: [%s]\n", DF_server_ip);
    printf("DF server Port: [%d]\n", DF_server_port);
    printf("\n");

    struct hostent *server = NULL;
    struct sockaddr_in address;

    DF_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (DF_server_socket < 0) {
        printf("ERROR: cannot open a socket !\n");
        exit(1);
    }

    server = gethostbyname(DF_server_ip);
    if (server == NULL) {
        printf("ERROR: cannot resolve hostname (%s) !\n", DF_server_ip);
        exit(1);
    }

    memset(&address, 0, sizeof(address));
    memcpy(&address.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    address.sin_family = AF_INET;
    address.sin_port = htons(DF_server_port);

    if (connect(DF_server_socket, (const struct sockaddr *) &address, sizeof(address)) < 0) {
        printf("ERROR: cannot connect with host (%s/%d) !\n", DF_server_ip, DF_server_port);
        exit(1);
    }

    printf("Connected to DF Server (%s/%d) .\n", DF_server_ip, DF_server_port);
}

void receiveMessage(INTERFACE_MESSAGE message) {
    TEAM1_INPUT_INTERFACE input;

    if (message.to == TEAM1) {
        input = message.input_interface.team1_input_interface;
        switch (message.from) {
        case TEAM2:
            printf("Message from TEAM2 to TEAM1 \n");
            ua_inputs.SignalFromTeam2 = input.SignalFromTeam2;
            break;
        default:
            printf("Received packet from unknown sender (%d) !\n", message.from);
            break ;
        }
    }
}

void * receiveMessagesFromPeers(void *ptr) {
    struct sockaddr_in address_sender;
    int reuseaddr_option = 1;

    from_outside_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (from_outside_socket < 0 ) {
        printf("ERROR: cannot open a socket !\n");
        exit(1);
    }

    /* This call is what allows broadcast packets to be received by many processes on same host */
    if (setsockopt(from_outside_socket, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_option, sizeof(reuseaddr_option)) < 0 ) {
        printf("ERROR: cannot configure socket for SO_REUSEADDR !\n");
        exit(1);
    }

    memset(&address_sender, 0, sizeof(address_sender));

    address_sender.sin_family = AF_INET;
    address_sender.sin_port = htons(DBUSPORT);
    address_sender.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(from_outside_socket, (struct sockaddr *) &address_sender, sizeof(address_sender)) < 0 ) {
        printf("ERROR: cannot bind socket !\n");
        exit(1);
    }

    int num_received_bytes;
    struct sockaddr_in address_receiver;
    int address_length = sizeof(struct sockaddr_in);

    char buffer[MSG_SIZE];

    while(1) {
        num_received_bytes = recvfrom(from_outside_socket, buffer, MSG_SIZE, MSG_WAIT_OPTION, (struct sockaddr *) &address_receiver, (socklen_t *) &address_length);
        if (num_received_bytes < 0) {
            printf("ERROR: could not receive messages from peers !\n");
            exit(1);
        }

        pthread_mutex_lock(&lock);
        receiveMessage(*(INTERFACE_MESSAGE*)buffer);
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

void connectToPeers() {
    int broadcast_option = 1;

    if ((!strcmp((char *) &to_outside_ip, "localhost")) || (!strcmp((char *) &to_outside_ip, "127.0.0.1")))
        gethostname((char *) &to_outside_ip, sizeof(to_outside_ip));

    if (gethostbyname(to_outside_ip) == NULL) {
        printf("ERROR: cannot resolve hostname (%s) !\n", to_outside_ip);
        exit(1);
    }

    to_outside_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (to_outside_socket < 0) {
        printf("ERROR: cannot open a socket !\n");
        exit(1);
    }

    if (setsockopt(to_outside_socket, SOL_SOCKET, SO_BROADCAST, &broadcast_option, sizeof(broadcast_option)) < 0) {
        printf("ERROR: cannot configure socket for SO_BROADCAST !\n");
        exit(1);
    }

    to_outside_address.sin_family = AF_INET;
    to_outside_address.sin_port = htons(DBUSPORT);
    to_outside_address.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    memset(to_outside_address.sin_zero, 0, sizeof(to_outside_address.sin_zero));

    pthread_attr_t attri;
    struct sched_param params;

    pthread_attr_init(&attri);
    pthread_attr_setinheritsched(&attri, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_getschedparam(&attri, &params);

    params.sched_priority = 20;
    pthread_attr_setschedparam(&attri, &params);

    pthread_create(&thread_receiver, &attri, receiveMessagesFromPeers, NULL);

    printf("Connected to Peers (%s) .\n", to_outside_ip);
}

void sendMessagesToPeers() {
    INTERFACE_MESSAGE message;

    message.to = TEAM2;
    message.from = TEAM1;
    TEAM2_INPUT_INTERFACE *output = &(message.input_interface.team2_input_interface);
    
    /*output->SignalFromTeam1 = ua_outputs.SignalToTeam2;*/
    
    sendto(to_outside_socket, (char *) &message, sizeof(message), MSG_WAIT_OPTION, (struct sockaddr *) &to_outside_address, sizeof(to_outside_address));

    /* Send messages to other peers */
}

void clear_ua_inputs() {
    ua_inputs.SignalFromTeam2 = FALSE;
}

void usage() {
    fprintf(stderr, "USAGE: Launch_team1UA.c <DF_server_ip> [<DF_server_port>]\n");
    exit(1);
}

void parseParameters(int argc, char **argv) {
    if (argc < 2)
        usage();
    if (argc >= 2)
        DF_server_ip = argv[1];
    if (argc >= 3)
        DF_server_port = atoi(argv[2]);
}

int main(int argc, char **argv) {
    parseParameters(argc, argv);

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("ERROR: cannot initialize mutex! \n");
        exit(1);
    }

    connectToDFServer();

    connectToPeers();

    unsigned long t1, t2;
    struct timespec clock;
    unsigned long sleep_time;

    char buffer[MSG_SIZE];
    int num_received_bytes;
    int num_sent_bytes;

    /* Start by resetting the inputs and outputs to their default values */
    pthread_mutex_lock(&lock);
    clear_ua_inputs();
    ua_receive_clear(&ua_inputs, NULL);
    project1_reset(&ua_outputs);
    pthread_mutex_unlock(&lock);

    while (1) {
        clock_gettime(CLOCK_REALTIME, &clock);
        t1 = clock.tv_sec * 1000000L;

        pthread_mutex_lock(&lock);

        /* Receive all input signals from DF server */
        num_received_bytes = recv(DF_server_socket, buffer, MSG_SIZE, MSG_DONTWAIT);
        if (num_received_bytes > 0)
            ua_receive((buffer_el *) buffer, num_received_bytes, &ua_inputs, NULL);

        /* Call the User Application generated by SCADE */
		project1(&ua_inputs, &ua_outputs);
        
        /* Clear inputs that are setted from outside */
        clear_ua_inputs();
        /* Clear inputs that are setted by CDS */
        ua_receive_clear(&ua_inputs, NULL);

        pthread_mutex_unlock(&lock);

        /* Send all output signals to DF server */
        num_sent_bytes = ua_send((buffer_el *) buffer, &ua_outputs, NULL);
        send(DF_server_socket, buffer, num_sent_bytes, MSG_DONTWAIT);

        /* Send messages to each peer */
        sendMessagesToPeers();

        clock_gettime(CLOCK_REALTIME, &clock);
        t2 = clock.tv_sec * 1000000L;

        sleep_time = 100000L;
        if (t2 - t1 > sleep_time)
            printf("Timing: cycle delayed by %d milliseconds! \n", (int) (((t2 - t1) - sleep_time)/1000L));
        else
            usleep(sleep_time - (t2 - t1));
    }

    pthread_join(thread_receiver, NULL);

    close(DF_server_socket);
    close(to_outside_socket);
    close(from_outside_socket);

    pthread_mutex_destroy(&lock);

    return 0;
}
