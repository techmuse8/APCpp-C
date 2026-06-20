#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef int AP_C_Bool;

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
    const char* itemName;
    const char* locationName;
    const char* playerName;
} AP_C_NetworkItem;

// all the custom containers that are converted from C++ STL equivalents
typedef struct AP_C_DataStorageOperation AP_C_DataStorageOperation;
typedef struct AP_C_DataStorageOperationVector {
    AP_C_DataStorageOperation* items;
    int size;
} AP_C_DataStorageOperationVector;

typedef struct AP_C_NetworkItemVector {
    AP_C_NetworkItem* items;
    int size;
} AP_C_NetworkItemVector;

// TODO: Make a generic macro for these?
typedef struct AP_C_StrVector {
    const char** items;
    int size;
} AP_C_StrVector;

typedef struct AP_C_IntIntPair {
    int key;
    int value;
} AP_C_IntIntPair;

typedef struct AP_C_MapIntInt {
    AP_C_IntIntPair* items;
    int size;
} AP_C_MapIntInt;

typedef struct AP_C_StrIntPair {
    const char* key;
    int value;
} AP_C_StrIntPair;

typedef struct AP_C_MapStrInt {
    AP_C_StrIntPair* items;
    int size;
} AP_C_MapStrInt;

typedef struct AP_C_StrStrPair {
    const char* key;
    const char* value;
} AP_C_StrStrPair;

typedef struct AP_C_MapStrStr {
    AP_C_StrStrPair* items;
    int size;
} AP_C_MapStrStr;

typedef struct AP_C_LocationArray {
    int64_t* items;
    size_t size;
} AP_C_LocationArray;

typedef struct AP_C_ItemArray {
    int64_t* items;
    size_t size;
} AP_C_ItemArray;

// "APC_DYNAMIC_MODE" lets you choose between either resolving all the APCpp-C
// function exports manually at runtime (LoadLibrary, dlsym, etc.) or link time 
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
DECL_APC_FUNCTION(void, AP_C_RegisterSlotDataMapIntIntCallback, const char* key, void (*f_slotdata)(AP_C_MapIntInt*));
DECL_APC_FUNCTION(void, AP_C_RegisterSlotDataRawCallback, const char* key, void (*f_slotdata)(const char*));

DECL_APC_FUNCTION(void, AP_C_SendLocationScouts, AP_C_LocationArray* locations, int create_as_hint);
DECL_APC_FUNCTION(void, AP_C_SetLocationInfoCallback, void (*f_locinfrecv)(AP_C_NetworkItemVector* items));

/* Game Management Functions */

DECL_APC_FUNCTION(void, AP_C_SendItem, int64_t location);
DECL_APC_FUNCTION(void, AP_C_SendItemSets, AP_C_ItemArray* itemArray);
DECL_APC_FUNCTION(void, AP_C_StoryComplete);

/* Deathlink Functions */

DECL_APC_FUNCTION(AP_C_Bool, AP_C_DeathLinkPending);
DECL_APC_FUNCTION(void, AP_C_DeathLinkClear);
DECL_APC_FUNCTION(void, AP_C_DeathLinkSend, const char* cause);

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

typedef struct AP_C_RoomInfo {
    AP_C_NetworkVersion version;
    AP_C_StrVector tags;
    AP_C_Bool password_required;
    AP_C_MapStrInt permissions;
    int hint_cost;
    int location_check_points;
    //MISSING: games
    AP_C_MapStrStr datapackage_checksums;
    const char* seed_name;
    double time;
} AP_C_RoomInfo;

/* Connection Information Functions */

DECL_APC_FUNCTION(int, AP_C_GetRoomInfo, AP_C_RoomInfo* client_roominfo);
DECL_APC_FUNCTION(AP_C_ConnectionStatus, AP_C_GetConnectionStatus);
DECL_APC_FUNCTION(uint64_t, AP_C_GetUUID);
DECL_APC_FUNCTION(int, AP_C_GetPlayerID);

/* Serverside Data Types */

typedef enum AP_C_RequestStatus {
    AP_ReqPending, AP_ReqDone, AP_ReqError
} AP_C_RequestStatus;

typedef enum AP_C_DataType {
    AP_TypeRaw, AP_TypeInt, AP_TypeDouble
} AP_C_DataType;

typedef struct AP_C_GetServerDataRequest {
    AP_C_RequestStatus status;
    const char* key;
    void* value;
    AP_C_DataType type;
} AP_C_GetServerDataRequest;

typedef struct AP_C_DataStorageOperation {
    const char* operation;
    void* value;
} AP_C_DataStorageOperation;

typedef struct AP_C_SetReply {
    const char* key;
    void* original_value;
    void* value;
} AP_C_SetReply;

typedef struct AP_C_SetServerDataRequest {
    AP_C_RequestStatus status;
    const char* key;
    AP_C_DataStorageOperationVector operations;
    void* default_value;
    AP_C_DataType type;
    AP_C_Bool want_reply;
} AP_C_SetServerDataRequest;

typedef struct AP_C_Bounce {
    AP_C_StrVector* games; // Can be nullptr or empty, but must be set to either
    AP_C_StrVector* slots; // Can be nullptr or empty, but must be set to either
    AP_C_StrVector* tags; // Can be nullptr or empty, but must be set to either
    const char* data; // Valid JSON Data. Can also be primitive (Numbers or literals)
} AP_C_Bounce;

typedef struct AP_C_StrAPTypePair {
    const char* key;
    AP_C_DataType value;
} AP_C_StrAPTypePair;

typedef struct AP_C_MapStrAPType {
    AP_C_StrAPTypePair* items;
    int size;
} AP_C_MapStrAPType;

/* Serverside Data Functions */

// Set and Receive Data
DECL_APC_FUNCTION(void, AP_C_SetServerData, AP_C_SetServerDataRequest* request);
DECL_APC_FUNCTION(void, AP_C_GetServerData, AP_C_GetServerDataRequest* request);

DECL_APC_FUNCTION(void, AP_C_BulkSetServerData, AP_C_SetServerDataRequest* requests);
DECL_APC_FUNCTION(void, AP_C_BulkGetServerData, AP_C_GetServerDataRequest* requests);

DECL_APC_FUNCTION(void, AP_C_CommitServerData);

DECL_APC_FUNCTION(const char*, AP_C_GetPrivateServerDataPrefix);

DECL_APC_FUNCTION(void, AP_C_RegisterSetReplyCallback, void (*f_setreply)(AP_C_SetReply*));

DECL_APC_FUNCTION(void, AP_C_SetNotify, AP_C_MapStrAPType* keylist, AP_C_Bool requestCurrentValue);
DECL_APC_FUNCTION(void, AP_C_SetNotifySingle, const char* key, AP_C_DataType type, AP_C_Bool requestCurrentValue);

DECL_APC_FUNCTION(void, AP_C_SendBounce, AP_C_Bounce* bounce);
DECL_APC_FUNCTION(void, AP_C_RegisterBouncedCallback, void (*f_bounced)(AP_C_Bounce*));

/* Gifting API Types */

struct AP_C_GiftBoxProperties {
    AP_C_Bool IsOpen;
    AP_C_Bool AcceptsAnyGift;
    AP_C_StrVector DesiredTraits;
};

struct AP_C_GiftTrait {
    const char* Trait;
    double Quality;  // 1. by default
    double Duration; // ^
};

typedef struct AP_C_GiftTraitVector {
    AP_C_GiftTrait* items;
    uint32_t size;
} AP_C_GiftTraitVector;

typedef struct AP_C_Gift {
    const char* ID;
    const char* ItemName;
    uint64_t Amount;
    uint64_t ItemValue;
    AP_C_GiftTraitVector Traits;
    const char* Sender;
    const char* Receiver;
    int SenderTeam; // Always 0 for now
    int ReceiverTeam; // Always 0 for now
    AP_C_Bool IsRefund;
} AP_C_Gift;

typedef struct AP_C_GiftVector {
    AP_C_Gift* items;
    int size;
} AP_C_GiftVector;

typedef struct AP_C_GiftBoxEntry {
    int key1;
    const char* key2;
    AP_C_GiftBoxProperties value;
} AP_C_GiftBoxEntry;

typedef struct AP_C_GiftBoxEntryArray {
    const AP_C_GiftBoxEntry* items;
    size_t size;
} AP_C_GiftBoxEntryArray;

typedef struct AP_C_GiftSet {
    const char** items;
    size_t size;
} AP_C_GiftSet;

/*
 * Gifting API Functions
 * 
 * These functions wrap around the DataStorage functions, but work in a blocking manner
 * They are only usable once authenticated. Be sure you are connected before using.
 * However, even if not all functions with possible data loss will report errors on connection loss.
 */

DECL_APC_FUNCTION(AP_C_RequestStatus, AP_C_SetGiftBoxProperties, AP_C_GiftBoxProperties* props);

DECL_APC_FUNCTION(AP_C_GiftBoxEntryArray, AP_C_QueryGiftBoxes);
DECL_APC_FUNCTION(AP_C_GiftVector*, AP_C_CheckGifts);

DECL_APC_FUNCTION(AP_C_RequestStatus, AP_C_AcceptGift, const char* id);
DECL_APC_FUNCTION(AP_C_RequestStatus, AP_C_AcceptGiftSet, AP_C_GiftSet* ids);

DECL_APC_FUNCTION(AP_C_RequestStatus, AP_C_RejectGift, const char* id);
DECL_APC_FUNCTION(AP_C_RequestStatus, AP_C_RejectGiftSet, AP_C_GiftSet* ids);

DECL_APC_FUNCTION(void, AP_C_UseGiftAutoReject, AP_C_Bool enable);
DECL_APC_FUNCTION(void, AP_C_SetGiftingSupported, AP_C_Bool enable);

#ifdef __cplusplus
}
#endif