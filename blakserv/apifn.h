#ifndef APIFN_H
#define APIFN_H
#include "blakserv.h"
#include "session.h"  // Needed for session_node

typedef struct {
    char name[32];
    int account_id;
    int version;
    int session_id;
    char connection[32];
    char state[32];
    int object_id;
} ApiWhoEntry;

typedef struct {
    char names[256][32];  // Store up to 256 players, max 32-char names
    int count;
} player_list_t;

void InitAPIServer();  // Initializes and starts the FastAPI server

player_list_t GetOnlinePlayers();  // Returns a list of online players

int ApiWho(ApiWhoEntry *out_list, int max_entries);  // Returns a list of all connected players

#endif
