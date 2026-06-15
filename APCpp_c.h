#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t AP_C_Bool;

typedef struct AP_C_NetworkVersion {
    int major;
    int minor;
    int build;
} AP_C_NetworkVersion;

typedef struct AP_C_NetworkItem {
    int64_t item;
    int64_t location;
    int player;
    int flags;
    char* itemName;
    char* locationName;
    char* playerName;
} AP_C_NetworkItem;

typedef struct AP_C_IntIntPair {
    int key;
    int value;
} AP_C_IntIntPair;

typedef struct AP_C_MapIntInt {
    AP_C_IntIntPair* items;
    int size;
} AP_C_MapIntInt;

void AP_C_SetClientVersion(AP_C_NetworkVersion* version);

void AP_C_Init(const char* ip, const char* game, const char* player_name, const char* passwd);
void AP_C_InitSinglePlayer(const char* filename);
AP_C_Bool AP_C_IsInit();
void AP_C_Start();
void AP_C_Shutdown();

/* Configuration Functions */

void AP_C_EnableQueueItemRecvMsgs(AP_C_Bool enable);
void AP_C_SetDeathLinkSupported(AP_C_Bool enable);

/* Required Callback Functions */

void AP_C_SetItemClearCallback(void (*cb)());
void AP_C_SetItemRecvCallback(void (*cb)(int64_t item, AP_C_Bool notify));
void AP_C_SetLocationCheckedCallback(void (*cb)(int64_t location));

/* Optional Callback Functions */

void AP_C_SetDeathLinkRecvCallback(void (*f_deathrecv)());
void AP_C_SetDeathLinkRecvCallbackEx(void (*f_deathrecv)(const char*, const char*));

// Parameter Function receives Slotdata of respective type
void AP_C_RegisterSlotDataIntCallback(const char* key, void (*f_slotdata)(int location));
void AP_C_RegisterSlotDataMapIntIntCallback(const char* key, void(*f_slotdata)(AP_C_MapIntInt*));
void AP_C_RegisterSlotDataRawCallback(const char* key, void (*f_slotdata)(const char*));

/* Game Management Functions */

void AP_C_SendItem(int64_t location);
//void AP_C_SendItemSets(int64_t location); // TODO: Implement this
void AP_C_StoryComplete();

/* Deathlink Functions */

AP_C_Bool AP_C_DeathLinkPending();
void AP_C_DeathLinkClear();
void AP_C_DeathLinkSend(const char* cause);

/* Message Management Types */
typedef enum AP_C_MessageType {
    AP_MsgPlaintext, AP_MsgItemSend, AP_MsgItemRecv, AP_MsgHint, AP_MsgCountdown
} AP_C_MessageType;

struct AP_C_Message {
    AP_C_MessageType type;
    const char* text;
};

/* Message Management Functions */

AP_C_Bool AP_C_IsMessagePending();
void AP_C_ClearLatestMessage();
AP_C_Message* AP_C_GetLatestMessage();

void AP_C_Say(const char* text);

/* Connection Information Types */

typedef enum AP_C_ConnectionStatus {
    AP_Disconnected, AP_Connected, AP_Authenticated, AP_ConnectionRefused
} AP_C_ConnectionStatus;

// #define AP_PERMISSION_DISABLED 0b000
// #define AP_PERMISSION_ENABLED 0b001
// #define AP_PERMISSION_GOAL 0b010
// #define AP_PERMISSION_AUTO 0b110

// struct AP_RoomInfo {
//     AP_NetworkVersion version;
//     std::vector<std::string> tags;
//     bool password_required;
//     std::map<std::string, int> permissions;
//     int hint_cost;
//     int location_check_points;
//     //MISSING: games
//     std::map<std::string, std::string> datapackage_checksums;
//     std::string seed_name;
//     double time;
// };

/* Connection Information Functions */

//int AP_GetRoomInfo(AP_RoomInfo*); //TODO: Implement
AP_C_ConnectionStatus AP_C_GetConnectionStatus();
uint64_t AP_C_GetUUID();
int AP_C_GetPlayerID();

#ifdef __cplusplus
}
#endif