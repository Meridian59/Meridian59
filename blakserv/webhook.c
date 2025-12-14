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

// Configuration constants
static const int MAX_WEBHOOK_PIPES = 10;

// Global state for persistent pipe connections
static bool webhook_initialized = false;
static char pipe_prefix[64] = "";
static int last_pipe_index = 0;
static HANDLE pipe_handles[MAX_WEBHOOK_PIPES];
static bool pipe_connected[MAX_WEBHOOK_PIPES];

// Helper functions
static void generate_pipe_name(int pipe_index, char *buffer, size_t buffer_size);
static void format_json_message(const char *message, int len, char *output, size_t output_size);
static HANDLE open_webhook_pipe(const char *pipe_name);
static bool write_webhook_pipe(HANDLE handle, const char *data, int len);

bool InitWebhooks(void)
{
    if (webhook_initialized) {
        return true;
    }

    // Check if webhooks are enabled in config
    if (!ConfigBool(WEBHOOK_ENABLED)) {
        return true; // Webhooks are disabled, nothing to initialize
    }

    // Store pipe prefix from config
    const char* config_prefix = ConfigStr(WEBHOOK_PREFIX);
    if (config_prefix && *config_prefix) {
        snprintf(pipe_prefix, sizeof(pipe_prefix), "%s_", config_prefix);
    } else {
        pipe_prefix[0] = '\0';
    }

    // Initialize pipe handles
    for (int i = 0; i < MAX_WEBHOOK_PIPES; i++) {
        pipe_handles[i] = INVALID_HANDLE_VALUE;
        pipe_connected[i] = false;
    }

    webhook_initialized = true;
    return true;
}

void ShutdownWebhooks(void)
{
    if (!webhook_initialized) {
        return;
    }

    // Close all pipe handles
    for (int i = 0; i < MAX_WEBHOOK_PIPES; i++) {
        if (pipe_connected[i] && pipe_handles[i] != INVALID_HANDLE_VALUE) {
            CloseHandle(pipe_handles[i]);
            pipe_handles[i] = INVALID_HANDLE_VALUE;
            pipe_connected[i] = false;
        }
    }

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

    // Try to send using round-robin across all pipes
    for (int i = 0; i < MAX_WEBHOOK_PIPES; i++) {
        int pipe_index = (last_pipe_index + i) % MAX_WEBHOOK_PIPES;
        
        // Try to connect if not already connected
        if (!pipe_connected[pipe_index]) {
            char pipe_name[128];
            generate_pipe_name(pipe_index + 1, pipe_name, sizeof(pipe_name));
            pipe_handles[pipe_index] = open_webhook_pipe(pipe_name);
            
            if (pipe_handles[pipe_index] != INVALID_HANDLE_VALUE) {
                pipe_connected[pipe_index] = true;
            }
        }
        
        // Try to send message
        if (pipe_connected[pipe_index]) {
            if (write_webhook_pipe(pipe_handles[pipe_index], message_to_send, json_len)) {
                last_pipe_index = (pipe_index + 1) % MAX_WEBHOOK_PIPES;
                return true;
            } else {
                // Write failed, disconnect
                CloseHandle(pipe_handles[pipe_index]);
                pipe_handles[pipe_index] = INVALID_HANDLE_VALUE;
                pipe_connected[pipe_index] = false;
            }
        }
    }

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

static HANDLE open_webhook_pipe(const char *pipe_name)
{
#ifdef BLAK_PLATFORM_WINDOWS
    return CreateFileA(pipe_name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
#else
    return open(pipe_name, O_WRONLY | O_NONBLOCK);
#endif
}

static bool write_webhook_pipe(HANDLE handle, const char *data, int len)
{
#ifdef BLAK_PLATFORM_WINDOWS
    DWORD bytes_written;
    BOOL success = WriteFile(handle, data, (DWORD)len, &bytes_written, NULL);
    return success && bytes_written == (DWORD)len;
#else
    ssize_t bytes_written = write(handle, data, len);
    return bytes_written == len;
#endif
}