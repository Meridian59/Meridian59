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
  - External webhook listeners create multiple pipe servers (m59apiwebhook1-10)
  - Each M59 server instance automatically claims an available pipe
  - Up to 10 concurrent servers supported per webhook listener instance
  - No configuration needed - automatic pipe distribution

  PERFORMANCE:
  - Pipes are connected on-demand and kept open for performance
  - Non-blocking operations prevent game server blocking

  PLATFORM SUPPORT:
  - Windows: Named pipes (e.g., \\\\.\\pipe\\m59apiwebhook1)
  - Linux/macOS: FIFO pipes (e.g., /tmp/m59apiwebhook1)
  
*/

#ifndef _WEBHOOK_H
#define _WEBHOOK_H

/**
 * Initialize the webhook system.
 * Prepares internal state for message delivery. Pipes are connected lazily on first use.
 * Should be called once during server startup.
 * 
 * Pipe prefix is controlled via [Webhook] Prefix in blakserv.cfg.
 * 
 * @return True if initialization successful, false otherwise
 */
bool InitWebhooks(void);

/**
 * Shutdown the webhook system.
 * Closes all open pipes and cleans up resources.
 * Should be called during server shutdown.
 */
void ShutdownWebhooks(void);

/**
 * Send a message via webhook pipes.
 * Plain text messages are wrapped in JSON with timestamp. Messages starting with '{'
 * are sent as-is (raw JSON). Uses round-robin across pipes for load distribution.
 * Non-blocking operation - returns false if no pipes are connected.
 * 
 * @param message The message content to send (plain text or JSON)
 * @param len Length of the message
 * @return True if message was sent successfully, false otherwise
 */
bool SendWebhookMessage(const char* message, int len);

bool IsWebhookEnabled(void);

#endif /* _WEBHOOK_H */