#include "APCpp_c.h"
#include "Archipelago.h"

static void (*c_item_clear_cb)() = nullptr;
static void (*c_item_recv_cb)(int64_t,AP_C_Bool) = nullptr;
static void (*c_loc_checked_cb)(int64_t) = nullptr;

static AP_C_Message sCurrentAPMessage;

extern "C" {

void AP_C_Init(const char *ip, const char *game, const char *player_name, const char *passwd) {
    AP_Init(ip, game, player_name, passwd);
}

AP_C_Bool AP_C_IsInit() {
    return AP_IsInit();
}

void AP_C_Start() {
    AP_Start();
}

void AP_C_Shutdown() {
    AP_Shutdown();
}

void AP_C_SetItemClearCallback(void (*cb)()) {
    c_item_clear_cb = cb;

    AP_SetItemClearCallback([](){
        if (c_item_clear_cb)
            c_item_clear_cb();
    });
}

void AP_C_SetItemRecvCallback(void (*cb)(int64_t,AP_C_Bool)) {
    c_item_recv_cb = cb;

    AP_SetItemRecvCallback([](int64_t item, bool notify){
        if (c_item_recv_cb)
            c_item_recv_cb(item, notify);
    });
}

void AP_C_SetLocationCheckedCallback(void (*cb)(int64_t)) {
    c_loc_checked_cb = cb;

    AP_SetLocationCheckedCallback([](int64_t loc){
        if (c_loc_checked_cb)
            c_loc_checked_cb(loc);
    });
}

void AP_C_EnableQueueItemRecvMsgs(AP_C_Bool enable) {
    AP_EnableQueueItemRecvMsgs(enable);
}

void AP_C_SetDeathLinkSupported(AP_C_Bool enable) {
    AP_SetDeathLinkSupported(enable);
}

void AP_C_SendItem(int64_t location) {
    AP_SendItem(location);
}

void AP_C_StoryComplete() {
    AP_StoryComplete();
}

AP_C_Bool AP_C_IsMessagePending() {
    return AP_IsMessagePending();
}

void AP_C_ClearLatestMessage() {
    AP_ClearLatestMessage();
}

AP_C_Message* AP_C_GetLatestMessage() {
    AP_Message* message = AP_GetLatestMessage();

    sCurrentAPMessage.text = message->text.c_str();
    sCurrentAPMessage.type = static_cast<AP_C_MessageType>(message->type);

    return &sCurrentAPMessage;
}

void AP_C_Say(const char* text) {
    AP_Say(text);
}


}