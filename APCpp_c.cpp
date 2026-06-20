#include "APCpp_c.h"
#include <Archipelago.h>
#include <cstring>
#include <map>
#include <vector>

static void (*c_deathlink_recvex_cb)(const char* source, const char* cause) = nullptr;
static void (*c_slot_data_int_cb)(int location) = nullptr;
static void (*c_slot_data_map_intint_cb)(AP_C_MapIntInt*) = nullptr;
static void (*c_slot_data_raw_cb)(const char*) = nullptr;
static void (*c_locinfrecv)(AP_C_NetworkItemVector *items) = nullptr;
static void (*c_setreply_cb)(AP_C_SetReply*) = nullptr;
static void (*c_bounce_cb)(AP_C_Bounce*) = nullptr;

static AP_C_Message sCurrentAPMessage;
static std::vector<AP_C_IntIntPair> sCurrentSlotDataMap;
static AP_C_RoomInfo sCurrentRoomInfo;

struct RoomInfoCtx {
    std::vector<AP_C_StrIntPair> permissionStorage;
    std::vector<AP_C_StrStrPair> dataPkgChecksums;
    std::vector<const char*> convertedTagStrings;
};

static RoomInfoCtx sRoomInfoCtx;

static std::string sCurrentServerKey;

// Storage for querying gift boxes
// TODO: Come up with better solution?
static std::vector<AP_C_GiftBoxEntry> sBoxes;
static std::vector<const char*> sTraitPtrs;
static std::vector<char> sStringArena;

struct currentGiftContext {
    std::vector<AP_C_Gift> currentGifts;
    std::vector<AP_C_GiftTrait> traits;
    AP_C_GiftVector userGiftVec;
};

static currentGiftContext sCurrentGiftContext;

namespace {

static const char* arenaAdd(const std::string& str) {
    size_t start = sStringArena.size();
    sStringArena.insert(sStringArena.end(), str.begin(), str.end());
    sStringArena.push_back('\0');
    return sStringArena.data() + start;
}


static inline const char* convert(const std::string& s) {
    return s.c_str();
}

template <typename T>
static inline T convert(const T& v) {
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
static void convertAPRoomInfo(const AP_RoomInfo& in, AP_C_RoomInfo &out) {
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

static void LocationInfoCallbackWrapper(std::vector<AP_NetworkItem> items) {
    AP_C_NetworkItemVector dstItems;
    std::vector<AP_C_NetworkItem> storage;
    
    storage.reserve(items.size());

    for (const auto& netItem : items) {
        storage.push_back({
            netItem.item,
            netItem.location,
            netItem.player,
            netItem.flags,
            netItem.itemName.c_str(),
            netItem.locationName.c_str(),
            netItem.playerName.c_str()
        });
    }

    dstItems.items = storage.data();
    dstItems.size = storage.size();

    c_locinfrecv(&dstItems);
}

static void SetReplyCallbackWrapper(AP_SetReply reply) {
    AP_C_SetReply dstReply;

    dstReply.key = reply.key.c_str();
    dstReply.original_value = reply.original_value;
    dstReply.value = reply.value;

    c_setreply_cb(&dstReply);
}

} // namespace

static void SetBounceCallbackWrapper(AP_Bounce bounce) {
    AP_C_Bounce dstBounce;

    AP_C_StrVector gameVec;
    AP_C_StrVector slotVec;
    AP_C_StrVector tagVec;

    std::vector<const char*> gameStorage;
    std::vector<const char*> slotStorage;
    std::vector<const char*> tagStorage;

    dstBounce.data = bounce.data.c_str();

    if (bounce.games)
        convertVector(*bounce.games, gameVec, gameStorage);
    if (bounce.slots)
        convertVector(*bounce.slots, slotVec, slotStorage);
    if (bounce.tags)
        convertVector(*bounce.tags, tagVec, tagStorage);

    dstBounce.games = &gameVec;
    dstBounce.slots = &slotVec;
    dstBounce.tags = &tagVec;

    c_bounce_cb(&dstBounce);

}

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

void AP_C_SetLocationInfoCallback(void (*f_locinfrecv)(AP_C_NetworkItemVector *items)) {
    c_locinfrecv = f_locinfrecv;

    AP_SetLocationInfoCallback(LocationInfoCallbackWrapper);
}

void AP_C_SendLocationScouts(AP_C_LocationArray* locations, int create_as_hint) {
    std::set<int64_t> locSet;

    for (size_t i = 0; i < locations->size; i++)
        locSet.insert(locations->items[i]);

    AP_SendLocationScouts(locSet, create_as_hint);
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

void AP_C_SendItemSets(AP_C_ItemArray* itemArray) {
    std::set<int64_t> locSet;

    for (size_t i = 0; i < itemArray->size; i++)
        locSet.insert(itemArray->items[i]);

    AP_SendItem(locSet);
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

void AP_C_SetServerData(AP_C_SetServerDataRequest* request) {
    AP_SetServerDataRequest inReq;

    inReq.status = static_cast<AP_RequestStatus>(request->status);
    inReq.key = request->key;
    inReq.default_value = request->default_value;
    inReq.type = static_cast<AP_DataType>(request->type);
    inReq.want_reply = request->want_reply;

    for (int i = 0; i < request->operations.size; i++) {
        AP_C_DataStorageOperation &entry = request->operations.items[i];
        inReq.operations.push_back({entry.operation, entry.value});
    }

    AP_SetServerData(&inReq);
    return;    
}

void AP_C_GetServerData(AP_C_GetServerDataRequest* request) {
    AP_GetServerDataRequest inReq;
    AP_GetServerData(&inReq);

    sCurrentServerKey = inReq.key;

    request->status = static_cast<AP_C_RequestStatus>(inReq.status);
    request->key = sCurrentServerKey.c_str();
    request->value = inReq.value;
    request->type = static_cast<AP_C_DataType>(inReq.type);

    return;
}

void AP_C_BulkSetServerData(AP_C_SetServerDataRequest* requests) {
    AP_SetServerDataRequest inReq;

    inReq.status = static_cast<AP_RequestStatus>(requests->status);
    inReq.key = requests->key;
    inReq.default_value = requests->default_value;
    inReq.type = static_cast<AP_DataType>(requests->type);
    inReq.want_reply = requests->want_reply;

    for (int i = 0; i < requests->operations.size; i++) {
        AP_C_DataStorageOperation &entry = requests->operations.items[i];
        inReq.operations.push_back({entry.operation, entry.value});
    }

    AP_BulkSetServerData(&inReq);
}

void AP_C_BulkGetServerData(AP_C_GetServerDataRequest* request) {
    AP_GetServerDataRequest inReq;
    AP_BulkGetServerData(&inReq);

    sCurrentServerKey = inReq.key;

    request->status = static_cast<AP_C_RequestStatus>(inReq.status);
    request->key = sCurrentServerKey.c_str();
    request->value = inReq.value;
    request->type = static_cast<AP_C_DataType>(inReq.type);

    return;
}

void AP_C_CommitServerData() {
    AP_CommitServerData();
}

const char* AP_C_GetPrivateServerDataPrefix() {
    return AP_GetPrivateServerDataPrefix().c_str();
}

void AP_C_RegisterSetReplyCallback(void (*f_setreply)(AP_C_SetReply*)) {
    c_setreply_cb = f_setreply;
    AP_RegisterSetReplyCallback(SetReplyCallbackWrapper);
}

void AP_C_SetNotify(AP_C_MapStrAPType* keylist, AP_C_Bool requestCurrentValue) {
    std::map<std::string, AP_DataType> inMap;

    for (int i = 0; i < keylist->size; i++) {
        AP_C_StrAPTypePair& entry = keylist->items[i];
        inMap.emplace(entry.key, static_cast<AP_DataType>(entry.value));
    }
    
    AP_SetNotify(inMap, requestCurrentValue);
}

void AP_C_SetNotifySingle(const char* key, AP_C_DataType type, AP_C_Bool requestCurrentValue) {
    AP_SetNotify(key, static_cast<AP_DataType>(type), requestCurrentValue);
}

void AP_C_SendBounce(AP_C_Bounce* bounce) {
    AP_Bounce inBounce;

    std::vector<std::string> gamesVec;
    std::vector<std::string> slotsVec;
    std::vector<std::string> tagsVec;

    inBounce.data = bounce->data;

    if (bounce->games) {
        for (int i = 0; i < bounce->games->size; i++) {
            const char* entry = bounce->games->items[i];
            gamesVec.push_back(entry);
        }
    }

    if (bounce->slots) {
        for (int i = 0; i < bounce->slots->size; i++) {
            const char* entry = bounce->slots->items[i];
            slotsVec.push_back(entry);
        }
    }

    if (bounce->tags) {
        for (int i = 0; i < bounce->tags->size; i++) {
            const char* entry = bounce->tags->items[i];
            tagsVec.push_back(entry);
        }
    }

    inBounce.games = &gamesVec;
    inBounce.slots = &slotsVec;
    inBounce.tags = &tagsVec;

    AP_SendBounce(inBounce);
    return;
}

void AP_C_RegisterBouncedCallback(void (*f_bounced)(AP_C_Bounce*)) {
    c_bounce_cb = f_bounced;

    AP_RegisterBouncedCallback(SetBounceCallbackWrapper);
}

AP_C_RequestStatus AP_C_SetGiftBoxProperties(AP_C_GiftBoxProperties* props) {
    AP_GiftBoxProperties inProps;
    
    inProps.IsOpen = props->IsOpen;
    inProps.AcceptsAnyGift = props->AcceptsAnyGift;

    for (int i = 0; i < props->DesiredTraits.size; i++) {
        const char* entry = props->DesiredTraits.items[i];
        inProps.DesiredTraits.push_back(entry);
    }

    return static_cast<AP_C_RequestStatus>(AP_SetGiftBoxProperties(inProps));
}

AP_C_GiftBoxEntryArray AP_C_QueryGiftBoxes() {
    auto boxes = AP_QueryGiftBoxes();

    sBoxes.clear();
    sTraitPtrs.clear();
    sStringArena.clear();

    sBoxes.reserve(boxes.size());

    for (const auto& [key, value] : boxes) {
        AP_C_GiftBoxProperties prop;
        prop.IsOpen = value.IsOpen;
        prop.AcceptsAnyGift = value.AcceptsAnyGift;

        const char* keyStr = arenaAdd(key.second);

        // Create a slice for each trait entry
        size_t traitStart = sTraitPtrs.size();

        for (const auto& trait : value.DesiredTraits) {
            sTraitPtrs.push_back(arenaAdd(trait.c_str()));
        }

        prop.DesiredTraits.items = sTraitPtrs.data() + traitStart;
        prop.DesiredTraits.size = sTraitPtrs.size() - traitStart;

        sBoxes.push_back({key.first, keyStr, prop});
    }

    return AP_C_GiftBoxEntryArray {
        sBoxes.data(),
        sBoxes.size()
    };
}

AP_C_GiftVector* AP_C_CheckGifts() {
    std::vector<AP_Gift> gifts = AP_CheckGifts();

    sCurrentGiftContext.currentGifts.clear();
    sCurrentGiftContext.traits.clear();

    // Great nested containers
    for (const auto& gift : gifts) {
        size_t traitStart = sCurrentGiftContext.traits.size();

        // Go a level deeper and save all the traits first
        for (const auto& trait : gift.Traits) {
            sCurrentGiftContext.traits.push_back({
            trait.Trait.c_str(),
            trait.Quality,
            trait.Duration
            });
        }

        size_t traitCount = sCurrentGiftContext.traits.size() - traitStart;

        AP_C_GiftTraitVector traitView {
            sCurrentGiftContext.traits.data() + traitStart,
            traitCount
        };

        AP_C_Gift cGift;
        cGift.ID = gift.ID.c_str();
        cGift.ItemName = gift.ItemName.c_str();
        cGift.Amount = gift.Amount;
        cGift.ItemValue = gift.ItemValue;
        cGift.Traits = traitView;
        cGift.Sender = gift.Sender.c_str();
        cGift.Receiver = gift.Receiver.c_str();
        cGift.SenderTeam = gift.SenderTeam;
        cGift.ReceiverTeam = gift.ReceiverTeam;
        cGift.IsRefund = gift.IsRefund;
        sCurrentGiftContext.currentGifts.push_back(cGift);
    }

    sCurrentGiftContext.userGiftVec.items = sCurrentGiftContext.currentGifts.data();
    sCurrentGiftContext.userGiftVec.size = sCurrentGiftContext.currentGifts.size();
    return &sCurrentGiftContext.userGiftVec;

}

AP_C_RequestStatus AP_C_AcceptGift(const char* id) {
    return static_cast<AP_C_RequestStatus>(AP_AcceptGift(id));
}

AP_C_RequestStatus AP_C_AcceptGiftSet(AP_C_GiftSet* ids) {
    std::set<std::string> giftSet;

    for (size_t i = 0; i < ids->size; i++)
        giftSet.insert(ids->items[i]);

    return static_cast<AP_C_RequestStatus>(AP_AcceptGift(giftSet));
}

AP_C_RequestStatus AP_C_RejectGift(const char* id) {
    return static_cast<AP_C_RequestStatus>(AP_RejectGift(id));
}

AP_C_RequestStatus AP_C_RejectGiftSet(AP_C_GiftSet* ids) {
    std::set<std::string> giftSet;

    for (size_t i = 0; i < ids->size; i++)
        giftSet.insert(ids->items[i]);

    return static_cast<AP_C_RequestStatus>(AP_RejectGift(giftSet));
}

void AP_C_UseGiftAutoReject(AP_C_Bool enable) {
    AP_UseGiftAutoReject(enable);
}

void AP_C_SetGiftingSupported(AP_C_Bool enable) {
    AP_SetGiftingSupported(enable);
}



}