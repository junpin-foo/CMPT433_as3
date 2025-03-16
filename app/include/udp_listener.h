/* udp_listener.h
 * This module provides functionality for receiving UDP commands to control the beat player.
 * It listens for incoming UDP packets and processes commands such as:
 * - "mode <value>" to set the current beat mode.
 * - "volume <value>" to adjust the volume.
 * - "tempo <value>" to set the tempo.
 * - "play <song_number>" to play a specific sound (e.g., Base Drum, Hi-Hat, Snare).
 * - "stop" to stop the beat player.
 * 
 * The module uses a separate thread to listen for commands and respond to the client.
 * UDP packets are received, parsed, and appropriate responses are sent back.
 * 
 * Functions:
 * - UdpListener_init() : Initializes the UDP socket and starts the listener thread.
 * - UdpListener_cleanup() : Cleans up resources by joining the listener thread and closing the socket.
 * - UdpListener_isRunning() : Checks if the UDP listener is still running.
 */

#ifndef _UDP_LISTENER_H_
#define _UDP_LISTENER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// start thread to listen for UDP messages
void UdpListener_init(void);

// clean up thread
void UdpListener_cleanup(void);

//return stop running flag
bool UdpListener_isRunning(void);

#endif