/* udp_listner.c
 * This file implements a UDP listener that listens for commands from a client and responds with the requested data.
 * The listener listens on port 12345 and supports the following commands:
 * - help: list of commands and summary
 * - count: Return the total number of light samples taken so far
 * - length: Return how many samples were captured during the previous second
 * - dips: Return how many dips were detected during the previous second’s samples
 * - history: Return all the data samples from the previous second
 * - stop: Exit the program
 * The listener runs in a separate thread and uses the Sampler module to get the required data.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "hal/rotary_encoder_statemachine.h"
#include "hal/pwm_rotary.h"
#include "hal/lcd.h"
#include <stdatomic.h> 
#include <assert.h>


#define PORT 12345
#define BUFFER_SIZE 1024
#define HELP_BUFFER_SIZE 512
#define SHORT_BUFFER_SIZE 64
#define MAX_UDP_BUFFER_SIZE 1500

static pthread_t udp_thread;
static int sockfd;
static struct sockaddr_in server_addr, client_addr;
static socklen_t addr_len = sizeof(client_addr);
static char *last_command = NULL;
static bool isInitialized = false;

static volatile atomic_bool running = true;

//Prototype
static void* udp_listener_thread(void* arg);
void UdpListener_init(void);
void UdpListener_cleanup(void);
bool UdpListener_isRunning(void);

void* udp_listener_thread(void* arg) {
    (void)arg; // Suppress unused parameter warning
    char buffer[BUFFER_SIZE];
    ssize_t received_len;

    while (running) {
        received_len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&client_addr, &addr_len);
        if (received_len < 0) {
            perror("Receive failed");
            continue;
        }
        if (received_len < BUFFER_SIZE) {
            buffer[received_len] = '\0';
            buffer[strcspn(buffer, "\r\n")] = '\0';  // Strip trailing newline or carriage return
        }

        if (strlen(buffer) == 0) {
            if (last_command == NULL) {
                sendto(sockfd, "Unknown command. Type 'help' for a list of commands.\n", 53, 0, (struct sockaddr*)&client_addr, addr_len);
                continue;
            }
            strncpy(buffer, last_command, sizeof(buffer) - 1); // Use the last valid command
            buffer[sizeof(buffer) - 1] = '\0';  // Ensure null-termination
        } else {
            free(last_command);
            last_command = strdup(buffer);  // Store last valid command
        }

        // printf("Received command: %s\n", buffer);

        if (strcmp(buffer, "help") == 0 || strcmp(buffer, "?") == 0) {
            char response[HELP_BUFFER_SIZE];
            snprintf(response, sizeof(response), 
                    "\nAccepted command examples:\n"
                    "count -- get the total number of samples taken.\n"
                    "length -- get the number of samples taken in the previously completed second.\n"
                    "dips -- get the number of dips in the previously completed second.\n"
                    "history -- get all the samples in the previously completed second.\n"
                    "stop -- cause the server program to end.\n"
                    "<enter> -- repeat last command.\n");

            sendto(sockfd, response, strlen(response), 0, (const struct sockaddr *)&client_addr, addr_len);
        } else {
            sendto(sockfd, "Unknown command. Type 'help' for a list of commands.\n", 53, 0, (struct sockaddr*)&client_addr, addr_len);
        }
    }
    return NULL;
}

void UdpListener_init(void) {
    assert(!isInitialized);
    isInitialized = true;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    pthread_create(&udp_thread, NULL, udp_listener_thread, NULL);
}

void UdpListener_cleanup(void) {
    assert(isInitialized);
    pthread_join(udp_thread, NULL);
    free(last_command);
    close(sockfd);
}

bool UdpListener_isRunning(void) {
    assert(isInitialized);

    return running;
}

