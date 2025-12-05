# Meridian 59 Webhook System

The Meridian 59 webhook system enables the game server to send real-time notifications to external services (like Discord) via named pipes or FIFO pipes. This system is designed for performance, reliability, and multi-server support.

**Note: Webhooks are disabled by default.** You must enable them in your server configuration.

## Quick Start

### 1. Enable Webhooks
Add this to your `blakserv.cfg` configuration file:
```ini
[Webhook]
Enabled              Yes
```

**Optional - For multiple servers on same machine:**
```ini
[Webhook]
Enabled              Yes
Prefix               server1
```

### 2. Install Webhook Listener
```bash
pip install m59api
```

### 3. Configure Discord Webhook (Required for Discord integration)
Set the Discord webhook URL environment variable:

**Windows PowerShell:**
```powershell
$env:DISCORD_WEBHOOK_URL = "https://discord.com/api/webhooks/your/webhook/url"
```

**Linux/macOS:**
```bash
export DISCORD_WEBHOOK_URL="https://discord.com/api/webhooks/your/webhook/url"
```

To get a Discord webhook URL: Server Settings ? Integrations ? Webhooks ? Create New Webhook

### 4. Start the Services (Order is Important!)
```bash
# Terminal 1: Start webhook listener FIRST
m59api webhook

# Terminal 2: Start your M59 server SECOND
./blakserv
```

**?? Important:** The webhook listener must be started before the M59 server, as the server connects to pipes created by the listener.

That's it! Game events will now appear in Discord.

## Architecture Overview

The webhook system uses a **client-server pipe architecture**:

- **Webhook Listener (Python)**: Creates and manages pipe servers, forwards messages to Discord/webhooks
- **Meridian 59 Server (C)**: Connects to pipes as a client, sends webhook messages

One example webhook listener is [m59api](https://pypi.org/project/m59api/), but any compatible pipe listener can be used.

```mermaid
graph LR
    subgraph "Game Events"
        A[Player Death]
        B[Player Login]  
        C[Chat Message]
    end
    
    subgraph "M59 Server (C)"
        D[SendWebhook Function]
        E[Webhook Subsystem]
    end
    
    subgraph "Pipe Communication"
        F[Named Pipes/FIFOs]
        F1[m59apiwebhook1]
        F2[m59apiwebhook2]
        F3[m59apiwebhook...]
        F --> F1
        F --> F2  
        F --> F3
    end
    
    subgraph "Webhook Listener (Python)"
        G[Pipe Listeners]
        H[Message Parser]
        I[Discord Formatter]
    end
    
    subgraph "External Services"
        J[Discord Webhook]
        K[Other Webhooks]
    end
    
    A --> D
    B --> D
    C --> D
    D --> E
    E --> F1
    E --> F2
    E --> F3
    F1 --> G
    F2 --> G
    F3 --> G
    G --> H
    H --> I
    I --> J
    I --> K
```

### Multi-Server Distribution

```mermaid
graph TB
    subgraph "Multiple M59 Servers"
        S1[M59 Server 1]
        S2[M59 Server 2]
        S3[M59 Server 3]
    end
    
    subgraph "Pipe Infrastructure"
        P1[m59apiwebhook1]
        P2[m59apiwebhook2]
        P3[m59apiwebhook3]
        P4[m59apiwebhook4-10]
    end
    
    subgraph "Webhook Listener Processing"
        L1[Listener 1]
        L2[Listener 2]
        L3[Listener 3]
        L4[Listeners 4-10]
    end
    
    subgraph "Discord"
        D[Discord Channel]
    end
    
    S1 -.->|claims| P1
    S2 -.->|claims| P2
    S3 -.->|claims| P3
    
    P1 --> L1
    P2 --> L2
    P3 --> L3
    P4 --> L4
    
    L1 --> D
    L2 --> D
    L3 --> D
    L4 --> D
    
    style S1 fill:#e1f5fe
    style S2 fill:#e8f5e8
    style S3 fill:#fff3e0
    style P1 fill:#e1f5fe
    style P2 fill:#e8f5e8
    style P3 fill:#fff3e0
```

## How It Works

### 1. Pipe Creation (webhook listener side)
When the webhook listener starts, it creates multiple named pipes to handle connections:

**Windows**: Creates 10 named pipes
- `\\.\pipe\m59apiwebhook1`  
- `\\.\pipe\m59apiwebhook2`
- ... up to `\\.\pipe\m59apiwebhook10`

**Linux/macOS**: Creates 10 FIFO pipes
- `/tmp/m59apiwebhook1`
- `/tmp/m59apiwebhook2`  
- ... up to `/tmp/m59apiwebhook10`

### 2. Connection (M59 server side)
When the Meridian 59 server starts:

1. **Initialization**: `InitWebhooks()` prepares the webhook subsystem
2. **Connection**: Server attempts to connect to available pipes in sequence
3. **Claiming**: First available pipe is "claimed" and kept open for performance
4. **Messaging**: `SendWebhookMessage()` writes to the claimed pipe
5. **Cleanup**: `ShutdownWebhooks()` closes connections on server shutdown

### 3. Message Flow
```
1. Game Event (e.g., player death)
2. Blakod calls SendWebhook("Player died: Bob killed by Alice")  
3. C_SendWebhook() ? SendWebhookMessage()
4. Format as JSON: {"timestamp":1234567890,"message":"Player died: Bob killed by Alice"}
5. Write to claimed pipe
6. Webhook listener receives message
7. Webhook listener forwards to Discord webhook
8. Discord posts to channel
```

## Multi-Server Support

The system automatically handles multiple M59 servers on the same machine:

### Automatic Pipe Distribution
- **Server 1**: Claims `m59apiwebhook1`
- **Server 2**: Finds `m59apiwebhook1` busy ? Claims `m59apiwebhook2`  
- **Server 3**: Finds `m59apiwebhook1,2` busy ? Claims `m59apiwebhook3`
- Up to **10 concurrent servers** supported per webhook listener instance

### Load Balancing
Each server uses round-robin selection across multiple pipes for better distribution and fault tolerance.

### High-Traffic Performance
The multi-pipe architecture prevents message dropping during high-traffic scenarios:

**Example - Multiple simultaneous events:**
```
Player 1 dies ? m59apiwebhook1 ? Concurrent processing
Player 2 dies ? m59apiwebhook2 ? Concurrent processing  
Player 3 dies ? m59apiwebhook3 ? Concurrent processing
All messages processed simultaneously without blocking
```

**Without multi-pipe design:**
- Single pipe processes one message at a time
- Subsequent messages could be blocked or dropped
- Performance degrades under load

## Performance Features

### Persistent Connections
- **Problem**: Opening/closing pipes for every message is expensive (3 system calls per message)
- **Solution**: Pipes are opened once during server startup and kept open
- **Result**: Only 1 system call per message (WriteFile/write)

### Non-blocking Operations  
- Pipe operations use `O_NONBLOCK` flags to prevent server blocking
- Failed pipe writes are handled gracefully without affecting game performance

## Usage

### Blakod Integration
```blakod
// Send a webhook message from Blakod
SendWebhook("Player login: " + GetPlayerName(player));
SendWebhook("Death message: " + killer + " killed " + victim);
```

### C Code Integration  
```c
// Send webhook message from C code
SendWebhookMessage("Server started", 14);
```

## Configuration

### Server Configuration File
Add this section to your `blakserv.cfg` configuration file:

```ini
[Webhook]
Enabled              Yes
Prefix               
```

**Configuration Options:**
- **Enabled**: `Yes` to enable webhooks, `No` to disable (default: `No`)
- **Prefix**: Optional prefix for pipe names (default: empty, use spaces for alignment)

### Basic Setup (Single Server)
1. Set `Enabled              Yes` in `blakserv.cfg` (note: use spaces, not `=`)
2. No other configuration needed - default pipe names work automatically

### Multi-Server Setup
If running multiple server instances:
1. Set `Enabled = Yes` in all server config files
2. No other configuration needed - automatic pipe distribution handles everything

### Custom Prefixes (Advanced)
For explicit control over pipe naming, set a prefix in config:

```ini
[Webhook]
Enabled = Yes
Prefix = server1
```

This creates pipes like `server1_m59apiwebhook1`, `server1_m59apiwebhook2`, etc.

For example, using m59api as the webhook listener:
```bash
pip install m59api
python -m m59api --webhook-prefix server1
python -m m59api --webhook-prefix server2  
```

## Implementation Details

### Files
- **webhook.h**: Function declarations and constants
- **webhook.c**: Cross-platform implementation
- **ccode.c**: Blakod function `C_SendWebhook()`
- **main.c**: Initialization and shutdown integration

### Key Functions
- `InitWebhooks(prefix)`: Initialize webhook system
- `ShutdownWebhooks()`: Cleanup resources  
- `SendWebhookMessage(message, len)`: Send message via pipe
- `C_SendWebhook()`: Blakod-accessible function

### Cross-Platform Support
The system uses `#ifdef BLAK_PLATFORM_WINDOWS` blocks to handle platform differences:

**Windows**: Named pipes via CreateFileA/WriteFile/CloseHandle
- Creates 10 concurrent pipe servers for high throughput
- Each pipe runs in its own thread for parallel processing

**Linux/macOS**: FIFO pipes via open/write/close  
- Creates 10 concurrent FIFO listeners for high throughput
- Each FIFO runs in its own async task for parallel processing

Both platforms now provide identical performance characteristics and multi-server support.

### Error Handling
- Graceful handling of unavailable pipes (webhook listener not running)
- Automatic retry and failover across multiple pipes
- No impact on game performance if webhook system is unavailable

## Troubleshooting

### Common Issues

**"No webhook messages appearing"**

**Required Setup Checklist:**
1. ? **blakserv.cfg configured**: Add `[Webhook]` section with `Enabled              Yes` 
2. ? **Discord webhook URL set**: Configure `DISCORD_WEBHOOK_URL` environment variable
3. ? **Webhook listener started FIRST**: Run `m59api webhook` before starting server
4. ? **Server recompiled**: Rebuild server with webhook code included
5. ? **Blakod calling webhooks**: Ensure game code calls `SendWebhook()` functions

**Common Errors:**
- `"No webhook URL set"`: Missing `DISCORD_WEBHOOK_URL` environment variable
- `"configstr found id 130 is dynamic"`: Need to recompile server with latest webhook code  
- `"Client not connected"`: Webhook listener not running or started after server

**"Multiple servers conflicting"**  
- System should handle this automatically
- Check that different servers are claiming different pipe numbers
- Consider using custom prefixes if needed

**"Performance issues"**
- Webhook system is designed for minimal performance impact
- 10 concurrent pipes handle high-traffic scenarios without blocking
- Messages are processed in parallel across multiple pipes/FIFOs
- If issues persist, webhook system can be disabled

### Debug Information
Enable webhook debugging by checking pipe connection status and message flow in server logs.

## Security Considerations

- Pipes are created with appropriate permissions
- Messages are sent locally only (no network access from M59 server)
- m59api handles all external network communication
- Webhook URLs should be kept secure
