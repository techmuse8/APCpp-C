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

// "APC_DYNAMIC_MODE" lets you choose between either resolving all the APCpp-C
// public functions manually at runtime (LoadLibrary, dlsym, etc.) or link time 
#ifdef APC_DYNAMIC_MODE
#define DECL_APC_FUNCTION(retType, func, ...)     extern retType (* func)(__VA_ARGS__)
#else
#define DECL_APC_FUNCTION(retType, func, ...)     retType func(__VA_ARGS__)
#endif

DECL_APC_FUNCTION(void, AP_C_SetClientVersion, AP_C_NetworkVersion* version);

DECL_APC_FUNCTION(void, AP_C_Init, const char* ip, const char* game, const char* player_name, const char* passwd);
DECL_APC_FUNCTION(void, AP_C_InitSinglePlayer, const char* filename);
DECL_APC_FUNCTION(AP_C_Bool, AP_C_IsInit);
DECL_APC_FUNCTION(void, AP_C_Start);
DECL_APC_FUNCTION(void, AP_C_Shutdown);

/* Configuration Functions */

DECL_APC_FUNCTION(void, AP_C_EnableQueueItemRecvMsgs, AP_C_Bool enable);
DECL_APC_FUNCTION(void, AP_C_SetDeathLinkSupported, AP_C_Bool enable);

/* Required Callback Functions */

DECL_APC_FUNCTION(void, AP_C_SetItemClearCallback, void (*cb)());
DECL_APC_FUNCTION(void, AP_C_SetItemRecvCallback, void (*cb)(int64_t item, AP_C_Bool notify));
DECL_APC_FUNCTION(void, AP_C_SetLocationCheckedCallback, void (*cb)(int64_t location));

/* Optional Callback Functions */

DECL_APC_FUNCTION(void, AP_C_SetDeathLinkRecvCallback, void (*f_deathrecv)());
DECL_APC_FUNCTION(void, AP_C_SetDeathLinkRecvCallbackEx, void (*f_deathrecv)(const char*, const char*));

// Parameter Function receives Slotdata of respective type
DECL_APC_FUNCTION(void, AP_C_RegisterSlotDataIntCallback, const char* key, void (*f_slotdata)(int location));
DECL_APC_FUNCTION(void, AP_C_RegisterSlotDataMapIntIntCallback, const char* key, void(*f_slotdata)(AP_C_MapIntInt*));
DECL_APC_FUNCTION(void, AP_C_RegisterSlotDataRawCallback, const char* key, void (*f_slotdata)(const char*));

/* Game Management Functions */

DECL_APC_FUNCTION(void, AP_C_SendItem, int64_t location);
//DECL_APC_FUNCTION(void, AP_C_SendItemSets, int64_t location); // TODO: Implement this
DECL_APC_FUNCTION(void, AP_C_StoryComplete);

/* Deathlink Functions */

DECL_APC_FUNCTION(AP_C_Bool, AP_C_DeathLinkPending);
DECL_APC_FUNCTION(void, AP_C_DeathLinkClear);
DECL_APC_FUNCTION(void, AP_C_DeathLinkSendconst, char* cause);

/* Message Management Types */
typedef enum AP_C_MessageType {
    AP_MsgPlaintext, AP_MsgItemSend, AP_MsgItemRecv, AP_MsgHint, AP_MsgCountdown
} AP_C_MessageType;

struct AP_C_Message {
    AP_C_MessageType type;
    const char* text;
};

/* Message Management Functions */

DECL_APC_FUNCTION(AP_C_Bool, AP_C_IsMessagePending);
DECL_APC_FUNCTION(void, AP_C_ClearLatestMessage);
DECL_APC_FUNCTION(AP_C_Message*, AP_C_GetLatestMessage);

DECL_APC_FUNCTION(void, AP_C_Say, const char* text);

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
DECL_APC_FUNCTION(AP_C_ConnectionStatus, AP_C_GetConnectionStatus);
DECL_APC_FUNCTION(uint64_t, AP_C_GetUUID);
DECL_APC_FUNCTION(int, AP_C_GetPlayerID);

#ifdef __cplusplus
}
#endif