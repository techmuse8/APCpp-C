#include "APCpp_c.h"
#include <Archipelago.h>
#include <cstring>
#include <map>
#include <vector>

static void (*c_deathlink_recvex_cb)(const char* source, const char* cause) = nullptr;
static void (*c_slot_data_int_cb)(int location) = nullptr;
static void (*c_slot_data_map_intint_cb)(AP_C_MapIntInt*) = nullptr;
static void (*c_slot_data_raw_cb)(const char*) = nullptr;

static AP_C_Message sCurrentAPMessage;
static std::vector<AP_C_IntIntPair> sCurrentSlotDataMap;
static AP_C_RoomInfo sCurrentRoomInfo;

struct RoomInfoCtx {
    std::vector<AP_C_StrIntPair> permissionStorage;
    std::vector<AP_C_StrStrPair> dataPkgChecksums;
    std::vector<const char*> convertedTagStrings;
};

static RoomInfoCtx sRoomInfoCtx;

namespace {

static inline const char* convert(const std::string& s) {
    return s.c_str();
}

template <typename T>
static inline T convert(const T& v)
{
    return v;
}


// convert std::map to C array view
template <typename Map, typename OutMap, typename Storage>
static void convertMap(const Map& sourceMap, OutMap& out, Storage& storageVec) {
    storageVec.clear();
    storageVec.reserve(sourceMap.size());

    for (const auto& [key, value] : sourceMap) {
        storageVec.push_back({convert(key), convert(value)});
    }

    out.items = storageVec.data();
    out.size = storageVec.size();
}

// convert std::vector to C array view
template <typename Vector, typename OutVector, typename Storage>
static void convertVector(const Vector& sourceVector, OutVector& out, Storage& storageVec) {
    storageVec.clear();
    storageVec.reserve(sourceVector.size());

    for (const auto& item : sourceVector) {
        storageVec.push_back({convert(item)});
    }

    out.items = storageVec.data();
    out.size = storageVec.size();
}

// Room info struct is owned internally
static void convertAPRoomInfo(AP_RoomInfo& in, AP_C_RoomInfo &out) {
    // version
    out.version.build = in.version.build;
    out.version.major = in.version.major;
    out.version.minor = in.version.minor;

    convertVector(in.tags, out.tags, sRoomInfoCtx.convertedTagStrings);

    out.password_required = in.password_required;
    convertMap(in.permissions, out.permissions, sRoomInfoCtx.permissionStorage);
    out.hint_cost = in.hint_cost;
    out.location_check_points = in.location_check_points;
    convertMap(in.datapackage_checksums, out.datapackage_checksums, sRoomInfoCtx.dataPkgChecksums);
    out.seed_name = in.seed_name.c_str();
    out.time = in.time;
}

static void SlotDataMapIntIntCallback(std::map<int,int> mapData) {
    AP_C_MapIntInt slotData;
    convertMap(mapData, slotData, sCurrentSlotDataMap);
    c_slot_data_map_intint_cb(&slotData);
}

} // namespace

extern "C" {

void AP_C_SetClientVersion(AP_C_NetworkVersion* version) {
    AP_NetworkVersion assignedVersion;
    
    assignedVersion.build = version->build;
    assignedVersion.major = version->major;
    assignedVersion.minor = version->minor;

    AP_SetClientVersion(&assignedVersion);

}

void AP_C_Init(const char *ip, const char *game, const char *player_name, const char *passwd) {
    AP_Init(ip, game, player_name, passwd);
}

void AP_C_InitSinglePlayer(const char* filename) {
    AP_Init(filename);
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
    AP_SetItemClearCallback(cb);
}

void AP_C_SetItemRecvCallback(void (*cb)(int64_t,AP_C_Bool)) {
    AP_SetItemRecvCallback(cb);
}

void AP_C_SetLocationCheckedCallback(void (*cb)(int64_t)) {
    AP_SetLocationCheckedCallback(cb);
}

void AP_C_SetDeathLinkRecvCallback(void (*f_deathrecv)()) {
   AP_SetDeathLinkRecvCallback(f_deathrecv);
}

void AP_C_SetDeathLinkRecvCallbackEx(void (*f_deathrecv)(const char* source, const char* cause)) {
    AP_SetDeathLinkRecvCallback([](std::string source, std::string cause){
        if (c_deathlink_recvex_cb)
            c_deathlink_recvex_cb(source.c_str(), cause.c_str());
    });
}

void AP_C_RegisterSlotDataIntCallback(const char* key, void (*f_slotdata)(int location)) {
    c_slot_data_int_cb = f_slotdata;

    AP_RegisterSlotDataIntCallback(key, [](int location){
        if (c_slot_data_int_cb)
            c_slot_data_int_cb(location);
    });
}

void AP_C_RegisterSlotDataMapIntIntCallback(const char* key, void(*f_slotdata)(AP_C_MapIntInt*)) {
    c_slot_data_map_intint_cb = f_slotdata;

    AP_RegisterSlotDataMapIntIntCallback(key, SlotDataMapIntIntCallback);
}

void AP_C_RegisterSlotDataRawCallback(const char* key, void (*f_slotdata)(const char*)) {
    c_slot_data_raw_cb = f_slotdata;

    AP_RegisterSlotDataRawCallback(key, [](std::string data){
        if (c_slot_data_raw_cb)
            c_slot_data_raw_cb(data.c_str());
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

AP_C_Bool AP_C_DeathLinkPending() {
    return AP_DeathLinkPending();
}

void AP_C_DeathLinkClear() {
    AP_DeathLinkClear();
}

void AP_C_DeathLinkSend(const char* cause) {
    AP_DeathLinkSend(cause);
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

int AP_C_GetRoomInfo(AP_C_RoomInfo* client_roominfo) {
    AP_RoomInfo srcRoominfo;
    int ret = AP_GetRoomInfo(&srcRoominfo);

    if (ret == 1)
        return ret;
    
    memset(&sCurrentRoomInfo, 0, sizeof(sCurrentRoomInfo));
    convertAPRoomInfo(srcRoominfo, sCurrentRoomInfo);
   *client_roominfo = sCurrentRoomInfo;

    return ret;
}

AP_C_ConnectionStatus AP_C_GetConnectionStatus() {
    return static_cast<AP_C_ConnectionStatus>(AP_GetConnectionStatus());
}

uint64_t AP_C_GetUUID() {
    return AP_GetUUID();
}

int AP_C_GetPlayerID() {
    return AP_GetPlayerID();
}

}