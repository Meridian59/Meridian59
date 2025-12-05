// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* webhook.h
*

  Cross-platform webhook message delivery system for Meridian 59 server.

  ARCHITECTURE:
  This system enables the M59 server to send real-time notifications (webhooks) to external
  services like Discord without blocking game performance. It uses a client-server pipe model:
  
  [M59 Server] --pipe--> [Webhook Listener] --HTTP--> [Discord/Webhooks]

  MULTI-SERVER SUPPORT:
  - External webhook listeners create multiple pipe servers (webhookN, where N=1-10)
  - Each M59 server instance automatically claims an available pipe
  - Up to 10 concurrent servers supported per webhook listener instance
  - No configuration needed - automatic pipe distribution

  PERFORMANCE:
  - Pipes are opened once at startup and kept open (not per-message)
  - Non-blocking operations prevent game server blocking
  - Only 1 system call per message vs 3 in naive implementation

  PLATFORM SUPPORT:
  - Windows: Named pipes (e.g., \\\\.\\pipe\\webhook1-10)
  - Linux/macOS: FIFO pipes (e.g., /tmp/webhook1-10)
  
*/

#ifndef _WEBHOOK_H
#define _WEBHOOK_H

/**
 * Initialize the webhook system.
 * Sets up pipe connections and prepares for message delivery.
 * Should be called once during server startup.
 * 
 * @param pipe_prefix Optional prefix for pipe names (e.g., "101" -> "101_m59apiwebhook1")
 *                   Pass NULL or empty string for default behavior ("m59apiwebhook1")
 *                   Default works for single-server setups, prefix needed for multi-server
 * @return True if initialization successful, false otherwise
 */
bool InitWebhooks(const char* pipe_prefix);

/**
 * Shutdown the webhook system.
 * Closes all open pipes and cleans up resources.
 * Should be called during server shutdown.
 */
void ShutdownWebhooks(void);

/**
 * Send a message via webhook pipes.
 * Formats the message as JSON with timestamp and sends to available pipes.
 * Non-blocking operation - if no pipes are available, message is dropped.
 * 
 * @param message The message content to send
 * @param len Length of the message
 * @return True if message was sent to at least one pipe, false otherwise
 */
bool SendWebhookMessage(const char* message, int len);



#endif /* _WEBHOOK_H */