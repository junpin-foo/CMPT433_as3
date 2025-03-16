#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdatomic.h> 
#include <stdbool.h>
#include <assert.h>
#include "beatPlayer.h"

#define PORT 12345
#define BUFFER_SIZE 1024
#define HELP_BUFFER_SIZE 512
#define SHORT_BUFFER_SIZE 64
#define MAX_UDP_BUFFER_SIZE 1500
#define DRUM_NUM 0 
#define HITHAT_NUM 1
#define SNARE_NUM 2

static pthread_t udp_thread;
static int sockfd;
static struct sockaddr_in server_addr, client_addr;
static socklen_t addr_len = sizeof(client_addr);
static bool isInitialized = false;

static bool running = true;

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

        // printf("Received command: %s\n", buffer);

        char response[BUFFER_SIZE];

        if (strncmp(buffer, "mode ", strlen("mode ")) == 0) {
            int mode;
            if (sscanf(buffer + strlen("mode "), "%d", &mode) == 1) {
                BeatPlayer_setBeatMode(mode);
                snprintf(response, sizeof(response), "%d", mode);
            } else if (strncmp(buffer + strlen("mode "), "null", strlen("null")) == 0) {
                snprintf(response, sizeof(response), "%d", BeatPlayer_getBeatMode());
            }
        } 
        if (strncmp(buffer, "volume ", strlen("volume ")) == 0) {
            int volume;
            if (sscanf(buffer + strlen("volume "), "%d", &volume) == 1) {
                snprintf(response, sizeof(response), "%d", volume);
                BeatPlayer_setVolume(volume);
            } else if (strncmp(buffer + strlen("volume "), "null", strlen("null")) == 0){
                snprintf(response, sizeof(response), "%d", BeatPlayer_getVolume());
            }
        } 
        if (strncmp(buffer, "tempo ", strlen("tempo ")) == 0) {
            int tempo;
            if (sscanf(buffer + strlen("tempo "), "%d", &tempo) == 1) {
                snprintf(response, sizeof(response), "%d", tempo);
                BeatPlayer_setBPM(tempo);
            } else if (strncmp(buffer + strlen("tempo "), "null", strlen("null")) == 0){
                snprintf(response, sizeof(response), "%d", BeatPlayer_getBpm());
            }
        } 
        if (strncmp(buffer, "play ", strlen("play ")) == 0) {
            int song;
            if (sscanf(buffer + strlen("play "), "%d", &song) == 1) {
                snprintf(response, sizeof(response), "%d", song);
                if(song == DRUM_NUM) {
                    BeatPlayer_playBaseDrum();
                }
                else if (song == HITHAT_NUM){
                    BeatPlayer_playHiHat();
                }
                else if (song == SNARE_NUM){
                    BeatPlayer_playSnare();
                }
            }
        } 
        if (strcmp(buffer, "stop") == 0) {
            snprintf(response, sizeof(response), "stop");
            sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)&client_addr, addr_len);
            running = false;  // Signal main thread to exit
            break;
        }
        // printf("Response: %s\n", response);
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)&client_addr, addr_len);
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
    close(sockfd);
}

bool UdpListener_isRunning(void) {
    assert(isInitialized);

    return running;
}

