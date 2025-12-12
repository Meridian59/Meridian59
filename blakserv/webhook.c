// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* webhook.c
*

  Cross-platform webhook message delivery via named pipes (Windows) or FIFOs (Linux/macOS).
  Keeps pipes open for performance instead of opening/closing on every message.

*/

#include "blakserv.h"
#include "webhook.h"
#include <time.h>

#ifdef BLAK_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

// Configuration constants
static const int MAX_WEBHOOK_PIPES_WINDOWS = 10;
static const int MAX_WEBHOOK_PIPES_UNIX = 10;

// Global state for persistent pipe connections
static bool webhook_initialized = false;
static char pipe_prefix[64] = "";
static int last_pipe_index = 0;

#ifdef BLAK_PLATFORM_WINDOWS
static HANDLE pipe_handles[10];
static bool pipe_connected[10];
static int num_pipes = MAX_WEBHOOK_PIPES_WINDOWS;
#else
static int pipe_fds[10];
static bool pipe_connected[10];
static int num_pipes = MAX_WEBHOOK_PIPES_UNIX;
#endif

// Helper functions
static void generate_pipe_name(int pipe_index, char *buffer, size_t buffer_size);
static void format_json_message(const char *message, int len, char *output, size_t output_size);

bool InitWebhooks(const char* prefix)
{
    if (webhook_initialized) {
        return true;
    }

    // Check if webhooks are enabled in config
    if (!ConfigBool(WEBHOOK_ENABLED)) {
        return false; // Webhooks disabled, return success but don't initialize
    }

    // Store pipe prefix (use config if none provided)
    const char* config_prefix = ConfigStr(WEBHOOK_PREFIX);
    if (prefix && *prefix) {
        snprintf(pipe_prefix, sizeof(pipe_prefix), "%s_", prefix);
    } else if (config_prefix && *config_prefix) {
        snprintf(pipe_prefix, sizeof(pipe_prefix), "%s_", config_prefix);
    } else {
        pipe_prefix[0] = '\0';
    }

#ifdef BLAK_PLATFORM_WINDOWS
    // Initialize Windows pipe handles
    for (int i = 0; i < num_pipes; i++) {
        pipe_handles[i] = INVALID_HANDLE_VALUE;
        pipe_connected[i] = false;
    }
#else
    // Initialize Unix pipes
    for (int i = 0; i < num_pipes; i++) {
        pipe_fds[i] = -1;
        pipe_connected[i] = false;
    }
#endif

    webhook_initialized = true;
    return true;
}

void ShutdownWebhooks(void)
{
    if (!webhook_initialized) {
        return;
    }

#ifdef BLAK_PLATFORM_WINDOWS
    // Close all Windows pipe handles
    for (int i = 0; i < num_pipes; i++) {
        if (pipe_connected[i] && pipe_handles[i] != INVALID_HANDLE_VALUE) {
            CloseHandle(pipe_handles[i]);
            pipe_handles[i] = INVALID_HANDLE_VALUE;
            pipe_connected[i] = false;
        }
    }
#else
    // Close all Unix pipes
    for (int i = 0; i < num_pipes; i++) {
        if (pipe_connected[i] && pipe_fds[i] >= 0) {
            close(pipe_fds[i]);
            pipe_fds[i] = -1;
            pipe_connected[i] = false;
        }
    }
#endif

    webhook_initialized = false;
}

bool SendWebhookMessage(const char* message, int len)
{
    if (!webhook_initialized || !message || len <= 0) {
        return false;
    }
    
    // Double-check config setting (in case it was changed dynamically)
    if (!ConfigBool(WEBHOOK_ENABLED)) {
        return false;
    }

    char json_message[1024];
    const char* message_to_send;
    int json_len;
    
    // If message already starts with '{', assume it's already JSON and skip wrapping
    if (message[0] == '{') {
        message_to_send = message;
        json_len = len;
    } else {
        // Legacy format: wrap plain text in JSON with timestamp
        format_json_message(message, len, json_message, sizeof(json_message));
        message_to_send = json_message;
        json_len = (int)strlen(json_message);
    }

#ifdef BLAK_PLATFORM_WINDOWS
    // Try to send to Windows pipes using round-robin
    for (int i = 0; i < num_pipes; i++) {
        int pipe_index = (last_pipe_index + i) % num_pipes;
        
        // Try to connect if not already connected
        if (!pipe_connected[pipe_index]) {
            char pipe_name[128];
            generate_pipe_name(pipe_index + 1, pipe_name, sizeof(pipe_name));
            pipe_handles[pipe_index] = CreateFileA(pipe_name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
            
            if (pipe_handles[pipe_index] != INVALID_HANDLE_VALUE) {
                pipe_connected[pipe_index] = true;
            }
        }
        
        // Try to send message
        if (pipe_connected[pipe_index]) {
            DWORD bytes_written;
            BOOL success = WriteFile(pipe_handles[pipe_index], message_to_send, (DWORD)json_len, &bytes_written, NULL);
            
            if (success && bytes_written == (DWORD)json_len) {
                last_pipe_index = (pipe_index + 1) % num_pipes;
                return true;
            } else {
                // Write failed, disconnect
                CloseHandle(pipe_handles[pipe_index]);
                pipe_handles[pipe_index] = INVALID_HANDLE_VALUE;
                pipe_connected[pipe_index] = false;
            }
        }
    }
#else
    // Unix/Linux implementation with round-robin like Windows
    for (int i = 0; i < num_pipes; i++) {
        int pipe_index = (last_pipe_index + i) % num_pipes;
        
        // Try to connect if not already connected
        if (!pipe_connected[pipe_index]) {
            char pipe_name[128];
            generate_pipe_name(pipe_index + 1, pipe_name, sizeof(pipe_name));
            pipe_fds[pipe_index] = open(pipe_name, O_WRONLY | O_NONBLOCK);
            
            if (pipe_fds[pipe_index] >= 0) {
                pipe_connected[pipe_index] = true;
            }
        }
        
        // Try to send message
        if (pipe_connected[pipe_index]) {
            ssize_t bytes_written = write(pipe_fds[pipe_index], message_to_send, json_len);
            
            if (bytes_written == json_len) {
                last_pipe_index = (pipe_index + 1) % num_pipes;
                return true;
            } else {
                // Write failed, disconnect
                close(pipe_fds[pipe_index]);
                pipe_fds[pipe_index] = -1;
                pipe_connected[pipe_index] = false;
            }
        }
    }
#endif

    return false;
}

static void generate_pipe_name(int pipe_index, char *buffer, size_t buffer_size)
{
#ifdef BLAK_PLATFORM_WINDOWS
    snprintf(buffer, buffer_size, "\\\\.\\pipe\\%sm59apiwebhook%d", pipe_prefix, pipe_index);
#else
    snprintf(buffer, buffer_size, "/tmp/%sm59apiwebhook%d", pipe_prefix, pipe_index);
#endif
}

static void format_json_message(const char *message, int len, char *output, size_t output_size)
{
    time_t now = time(NULL);
    
    // Truncate message if too long
    int max_msg_len = (int)(output_size - 50);
    if (len > max_msg_len) {
        len = max_msg_len;
    }

    snprintf(output, output_size, "{\"timestamp\":%ld,\"message\":\"%.*s\"}", (long)now, len, message);
}