#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t AP_C_Bool;

struct AP_C_NetworkVersion {
    int major;
    int minor;
    int build;
};

struct AP_C_NetworkItem {
    int64_t item;
    int64_t location;
    int player;
    int flags;
    char* itemName;
    char* locationName;
    char* playerName;
};

enum AP_C_MessageType {
    Plaintext, ItemSend, ItemRecv, Hint, Countdown
};

void AP_C_Init(const char* ip, const char* game, const char* player_name, const char* passwd);
AP_C_Bool AP_C_IsInit();
void AP_C_Start();
void AP_C_Shutdown();

void AP_C_SetItemClearCallback(void (*cb)());
void AP_C_SetItemRecvCallback(void (*cb)(int64_t item, AP_C_Bool notify));
void AP_C_SetLocationCheckedCallback(void (*cb)(int64_t location));

void AP_C_EnableQueueItemRecvMsgs(AP_C_Bool enable);
void AP_C_SetDeathLinkSupported(AP_C_Bool enable);

void AP_C_SendItem(int64_t location);
void AP_C_StoryComplete();

AP_C_Bool AP_C_IsMessagePending();
void AP_C_ClearLatestMessage();

struct AP_C_Message {
    AP_C_MessageType type;
    const char* text;
};

AP_C_Message* AP_C_GetLatestMessage();

void AP_C_Say(const char* text);


#ifdef __cplusplus
}
#endif