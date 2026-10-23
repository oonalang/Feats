#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Fields.h"

namespace OblivionSkinHax {
static constexpr uintptr_t kItemInventoryCtor   = 0x6971D34;
static constexpr uintptr_t kWeaponConfCtor      = 0xAF7E7FC;
static constexpr uintptr_t kWeaponExtraCtor     = 0xAF80080;
static constexpr uintptr_t kWeaponFireCtor      = 0xAF81990;
static constexpr uintptr_t kWeaponAssetCtor     = 0xAF7A080;
static constexpr uintptr_t kMythicArmorCtor     = 0x5C90E84;
static constexpr uintptr_t kMythicSightCtor     = 0x90DDEEC;
static constexpr uintptr_t kKillEffectCtor      = 0x90C54CC;
static constexpr uintptr_t kWeaponSkinCtor      = 0xAF8B4D4;
static constexpr uintptr_t kItemResourceCtor    = 0x6985AB8;
static constexpr uintptr_t kWeaponNameGetter    = 0xAF7FDC8;
static constexpr uintptr_t kWeaponPartAssetCtor = 0xAF85628;
static constexpr uintptr_t kAttachmentConfCtor  = 0x6FE9168;

static constexpr uintptr_t kCurrentKillEffect  = 0xA61AC70;
static constexpr uintptr_t kCurrentFireEffect  = 0xA61AAA8;
static constexpr uintptr_t kCurrentBulletTrack = 0xA61AB8C;
static constexpr uintptr_t kCurrentBroadcast   = 0xA61ADFC;

static constexpr uintptr_t kAttachmentGroupNewCtor       = 0x8719E4C;
static constexpr uintptr_t kAttachmentGroupNewItemCtor   = 0x871A270;
static constexpr uintptr_t kAttachmentGroupNewGetColorId = 0x871A5D8;
static constexpr uintptr_t kBRWeaponAttachItemDataCtor   = 0x99811C0;
static constexpr uintptr_t kAttachItemCtor               = 0x8449500;
static constexpr uintptr_t kAttachItemSetData            = 0x8449544;
static constexpr uintptr_t kAttachItemSetMythicSkinId    = 0x844988C;
static constexpr uintptr_t kAttachItemCopyEquipped       = 0x84498F0;
static constexpr uintptr_t kAttachItemGetModelAttachId   = 0x844A18C;
static constexpr uintptr_t kAttachSkinDataCtor           = 0x75D7DE4;
static constexpr uintptr_t kLoadoutAttachItemSetData     = 0xB83F1C0;
static constexpr uintptr_t kWeaponInitPart               = 0x517C4D0;

using Constructor = void (*)(void*);
using EffectGetter = int (*)(void*);

// ============================================================================
// Shared state
// ============================================================================
inline std::mutex gInstanceMutex;
inline std::mutex gDatabaseMutex;
inline std::mutex gSelectionMutex;
inline std::mutex gEffectMutex;
inline std::mutex gMythicCollectorMutex;
inline std::shared_mutex gSnapshotMutex;

inline std::vector<void*> gInventoryInstances;
inline std::vector<void*> gExtraInstances;
inline std::vector<void*> gFireInstances;
inline std::vector<void*> gConfInstances;
inline std::vector<void*> gAssetInstances;
inline std::vector<void*> gArmorInstances;
inline std::vector<void*> gSightInstances;
inline std::vector<void*> gKillInstances;
inline std::vector<void*> gResourceInstances;
inline std::vector<void*> gWeaponPartAssetInstances;
inline std::vector<void*> gWeaponSkinConfigInstances;
inline std::vector<void*> gAttachmentConfInstances;

inline Constructor gOrigInventory = nullptr;
inline Constructor gOrigExtra = nullptr;
inline Constructor gOrigFire = nullptr;
inline Constructor gOrigConf = nullptr;
inline Constructor gOrigAsset = nullptr;
inline Constructor gOrigArmor = nullptr;
inline Constructor gOrigSight = nullptr;
inline Constructor gOrigKill = nullptr;
inline Constructor gOrigWeaponSkin = nullptr;
inline Constructor gOrigResource = nullptr;
inline Constructor gOrigWeaponPartAsset = nullptr;
inline Constructor gOrigAttachmentConf = nullptr;

inline EffectGetter gOrigKillEffect = nullptr;
inline EffectGetter gOrigFireEffect = nullptr;
inline EffectGetter gOrigBulletTrack = nullptr;
inline EffectGetter gOrigBroadcast = nullptr;

inline std::unordered_map<int, int> gActiveKillEffects;
inline std::unordered_map<int, int> gActiveFireEffects;
inline std::unordered_map<int, int> gActiveBulletEffects;
inline std::unordered_map<int, int> gActiveBroadcastEffects;

inline std::atomic<bool> gInstalled{false};
inline std::atomic<bool> gRuntimeInstalled{false};
inline std::atomic<bool> gWorkerStarted{false};
inline std::atomic<bool> gDatabaseReady{false};
inline std::atomic<bool> gScanRequested{false};
inline size_t gLastUiBuildFingerprint = 0;
inline bool gHasSelectedSkin = false;
inline itemInfo gSelectedSkin{};
inline std::string gSelectedName;
inline int gRarityFilter = 1;
inline bool gSearchKeyboard = false;

inline std::unordered_set<void*> gSeen;
inline std::atomic<size_t> gFingerprintCount{0};
inline std::atomic<uint64_t> gDbVersion{0};
inline std::atomic<uint64_t> gCaptureVersion{0};

// ============================================================================
// Mythic attachment runtime state
// ============================================================================
inline std::unordered_map<int, int> gActiveWeaponSkins;
inline std::unordered_map<int, std::array<int, 31>> gActiveMythicAttachments;
inline std::array<int, 31> gActiveMythicAttachFallback{};
inline std::unordered_map<uint, uint> gRuntimeAttachMythicById;
inline std::unordered_map<uint64_t, int> gRuntimeAttachResolveCache;
inline std::unordered_map<uint64_t, bool> gLoadoutPreviewTiles;
inline std::unordered_map<uint, int> gAttachmentTypeCache;
inline std::unordered_map<uint64_t, void*> gWeaponPartConfCache;
inline size_t gAttachTypeIndexed = 0;
inline size_t gPartConfIndexed = 0;
inline uint gRuntimeAttachWeaponId = 0;
inline uint gRuntimeAttachSkinId = 0;

// ============================================================================
// Guards and helpers
// ============================================================================
static bool Valid(void* p) {
    return p && Tools::IsPtrValid(p);
}

inline bool AttachPtr(const void* ptr) {
    return (uintptr_t)ptr >= 0x10000;
}

static void Capture(std::vector<void*>& out, void* instance) {
    if (!Valid(instance)) return;
    std::lock_guard<std::mutex> lock(gInstanceMutex);
    if (gSeen.insert(instance).second) {
        out.push_back(instance);
        gFingerprintCount.fetch_add(1, std::memory_order_relaxed);
        gCaptureVersion.fetch_add(1, std::memory_order_release);
    }
}

static void HookInventory(void* p) { if (gOrigInventory) gOrigInventory(p); Capture(gInventoryInstances, p); }
static void HookExtra(void* p)     { if (gOrigExtra)     gOrigExtra(p);     Capture(gExtraInstances, p); }
static void HookFire(void* p)      { if (gOrigFire)      gOrigFire(p);      Capture(gFireInstances, p); }
static void HookConf(void* p)      { if (gOrigConf)      gOrigConf(p);      Capture(gConfInstances, p); }
static void HookAsset(void* p)     { if (gOrigAsset)     gOrigAsset(p);     Capture(gAssetInstances, p); }
static void HookArmor(void* p)     { if (gOrigArmor)     gOrigArmor(p);     Capture(gArmorInstances, p); }
static void HookSight(void* p)     { if (gOrigSight)     gOrigSight(p);     Capture(gSightInstances, p); }
static void HookKill(void* p)      { if (gOrigKill)      gOrigKill(p);      Capture(gKillInstances, p); }
static void HookResource(void* p)  { if (gOrigResource)  gOrigResource(p);  Capture(gResourceInstances, p); }

static void HookWeaponSkin(void* p) {
    if (gOrigWeaponSkin) gOrigWeaponSkin(p);
    if (!AttachPtr(p)) return;
    std::lock_guard<std::mutex> lock(gMythicCollectorMutex);
    if (std::find(gWeaponSkinConfigInstances.begin(),
                  gWeaponSkinConfigInstances.end(), p) ==
        gWeaponSkinConfigInstances.end()) {
        gWeaponSkinConfigInstances.push_back(p);
        gCaptureVersion.fetch_add(1, std::memory_order_release);
    }
}

static void HookWeaponPartAsset(void* p) {
    if (gOrigWeaponPartAsset) gOrigWeaponPartAsset(p);
    if (!AttachPtr(p)) return;
    std::lock_guard<std::mutex> lock(gMythicCollectorMutex);
    if (std::find(gWeaponPartAssetInstances.begin(),
                  gWeaponPartAssetInstances.end(), p) ==
        gWeaponPartAssetInstances.end()) {
        gWeaponPartAssetInstances.push_back(p);
        gCaptureVersion.fetch_add(1, std::memory_order_release);
    }
}

static void HookAttachmentConf(void* p) {
    if (gOrigAttachmentConf) gOrigAttachmentConf(p);
    if (!AttachPtr(p)) return;
    std::lock_guard<std::mutex> lock(gMythicCollectorMutex);
    if (std::find(gAttachmentConfInstances.begin(),
                  gAttachmentConfInstances.end(), p) ==
        gAttachmentConfInstances.end()) {
        gAttachmentConfInstances.push_back(p);
        gCaptureVersion.fetch_add(1, std::memory_order_release);
    }
}

inline const std::vector<void*>& WeaponSkinConfigRef() {
    return gWeaponSkinConfigInstances;
}

inline const std::vector<void*>& WeaponPartAssetRef() {
    return gWeaponPartAssetInstances;
}

// Legacy aliases — still used in a couple of places for compatibility.
inline const std::vector<void*>& SnapshotWeaponSkinConfigs() { return WeaponSkinConfigRef(); }
inline const std::vector<void*>& SnapshotWeaponPartAssets()  { return WeaponPartAssetRef(); }

struct Snapshot {
    std::vector<void*> inventory, extra, fire, conf, asset;
    std::vector<void*> armor, sight, kill, resource;
};

// Thread-local cached snapshot, rebuilt only when gCaptureVersion changes.
inline const Snapshot& GetSnapshotRef() {
    static thread_local Snapshot cached;
    static thread_local uint64_t cachedVersion = ~0ull;
    const uint64_t v = gCaptureVersion.load(std::memory_order_acquire);
    if (cachedVersion != v) {
        std::lock_guard<std::mutex> lock(gInstanceMutex);
        cached = {gInventoryInstances, gExtraInstances, gFireInstances,
                  gConfInstances, gAssetInstances, gArmorInstances,
                  gSightInstances, gKillInstances, gResourceInstances};
        cachedVersion = v;
    }
    return cached;
}

static size_t Fingerprint() {
    return gFingerprintCount.load(std::memory_order_acquire)
         + gWeaponSkinConfigInstances.size();
}

static const char* RarityPrefix(int colorId) {
    switch (colorId) {
        case 5: return "[M] ";
        case 4: return "[L] ";
        case 3: return "[E] ";
        default: return "[C] ";
    }
}

static int RarityFromName(const std::string& name) {
    if (name.rfind("[M] ", 0) == 0) return 1;
    if (name.rfind("[L] ", 0) == 0) return 2;
    if (name.rfind("[E] ", 0) == 0) return 3;
    return 4;
}

static const char* RarityName(int rarity) {
    switch (rarity) {
        case 1: return "MYTHIC";
        case 2: return "LEGENDARY";
        case 3: return "EPIC";
        default: return "COMMON";
    }
}

static ImVec4 RarityColor(int rarity) {
    switch (rarity) {
        case 1: return ImVec4(0.95f, 0.16f, 0.18f, 1.00f);
        case 2: return ImVec4(0.98f, 0.78f, 0.12f, 1.00f);
        case 3: return ImVec4(0.66f, 0.31f, 0.92f, 1.00f);
        case 4: return ImVec4(0.22f, 0.80f, 0.38f, 1.00f);
        default: return ImVec4(0.72f, 0.76f, 0.82f, 1.00f);
    }
}

static ImVec4 WithAlpha(ImVec4 c, float a) {
    c.w = a;
    return c;
}

static bool DrawRarityTabBox(const char* label, int rarity, bool selected, const ImVec2& size) {
    const ImVec4 color = RarityColor(rarity);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::PushStyleColor(ImGuiCol_Button, WithAlpha(color, selected ? 0.28f : 0.10f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, WithAlpha(color, 0.24f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, WithAlpha(color, 0.34f));
    ImGui::PushStyleColor(ImGuiCol_Border, WithAlpha(color, selected ? 0.95f : 0.60f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    const bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(5);
    return pressed;
}

static int ArrayLogicalAt(Array<int>* a, int i) {
    if (!Valid(a)) return 0;
    const int len = a->getLength();
    if (i < 0 || i >= len || len > 4096) return 0;
    return a->m_Items[i + 1];
}

static int ArrayLogicalLast(Array<int>* a) {
    if (!Valid(a) || a->getLength() <= 0) return 0;
    return ArrayLogicalAt(a, a->getLength() - 1);
}

inline void IndexAttachmentTypes() {
    if (gAttachmentConfInstances.size() == gAttachTypeIndexed) return;
    std::lock_guard<std::mutex> lock(gMythicCollectorMutex);
    for (; gAttachTypeIndexed < gAttachmentConfInstances.size(); ++gAttachTypeIndexed) {
        void* conf = gAttachmentConfInstances[gAttachTypeIndexed];
        if (!AttachPtr(conf)) continue;
        const uint id = *(uint*)((uintptr_t)conf + 0x68);
        if (id > 0)
            gAttachmentTypeCache[id] = *(uint8_t*)((uintptr_t)conf + 0x40);
    }
}

inline int GetAttachmentType(uint attachId) {
    if (gAttachmentTypeCache.empty() || gAttachmentConfInstances.size() != gAttachTypeIndexed)
        IndexAttachmentTypes();
    auto found = gAttachmentTypeCache.find(attachId);
    return found != gAttachmentTypeCache.end() ? found->second : 0;
}

inline const std::array<int, 9>& MythicAttachTypes() {
    static const std::array<int, 9> types = {1, 2, 3, 4, 5, 6, 7, 8, 10};
    return types;
}

inline int AttachSlotForType(int attachType) {
    const auto& types = MythicAttachTypes();
    for (int i = 0; i < (int)types.size(); ++i)
        if (types[i] == attachType) return i;
    return -1;
}

inline bool IsRealMythicAttachSkin(uint mythicId) {
    return mythicId >= 600000000 && mythicId < 700000000;
}

inline int RealMythicFromArray(Array<int>* ids, int slot) {
    const int len = AttachPtr(ids) ? ids->getLength() : 0;
    if (slot < 0 || len <= slot || len > 64) return 0;
    const int mythicId = ids->m_Items[slot + 1];
    return IsRealMythicAttachSkin((uint)mythicId) ? mythicId : 0;
}

inline int RealMythicForType(Array<int>* ids, int attachType) {
    if (!AttachPtr(ids) || attachType <= 0) return 0;
    return RealMythicFromArray(ids, AttachSlotForType(attachType));
}

inline int RealMythicForType(WeaponSkinFields* fields, int attachType) {
    if (!AttachPtr(fields)) return 0;
    const int skin1P = RealMythicForType(fields->SkinID1P, attachType);
    if (skin1P > 0) return skin1P;
    return RealMythicForType(fields->SkinID3P, attachType);
}

inline int ResolveRealMythicAttach(uint weaponId, int attachType) {
    if (weaponId == 0 || attachType <= 0) return 0;

    for (void* weaponSkin : WeaponSkinConfigRef()) {
        if (!AttachPtr(weaponSkin)) continue;
        auto* fields = (WeaponSkinFields*)((uintptr_t)weaponSkin + 0x10);
        const bool match =
            fields->ID == weaponId ||
            fields->WeaponID == weaponId ||
            fields->BRWeaponID == weaponId ||
            (uint)fields->BrSkinWeaponID == weaponId;
        if (!match) continue;
        const int mythicId = RealMythicForType(fields, attachType);
        if (mythicId > 0) return mythicId;
    }

    const uint weaponIds[] = {gRuntimeAttachSkinId, weaponId};
    for (uint id : weaponIds) {
        if (id == 0) continue;
        for (void* part : WeaponPartAssetRef()) {
            if (!AttachPtr(part)) continue;
            const int partType    = *(int*)((uintptr_t)part + 0x14);
            const uint partItemId = *(uint*)((uintptr_t)part + 0x58);
            const uint partWeaponId = *(uint*)((uintptr_t)part + 0x7C);
            if (partWeaponId == id && partType == attachType &&
                IsRealMythicAttachSkin(partItemId)) {
                return (int)partItemId;
            }
        }
    }
    return 0;
}

inline int ActiveMythicAttachFor(uint weaponId, int attachType) {
    if (attachType <= 0 || attachType >= 31) return 0;
    if (gActiveMythicAttachments.empty()) return 0;

    auto active = gActiveMythicAttachments.find((int)weaponId);
    if (active != gActiveMythicAttachments.end() && active->second[attachType] > 0) {
        const int selected = active->second[attachType];
        if (IsRealMythicAttachSkin((uint)selected)) return selected;
        const int resolved = ResolveRealMythicAttach(weaponId, attachType);
        if (resolved > 0) return resolved;
        return selected;
    }

    const uint64_t key = ((uint64_t)weaponId << 8) | (uint64_t)attachType;
    auto cached = gRuntimeAttachResolveCache.find(key);
    if (cached != gRuntimeAttachResolveCache.end()) return cached->second;

    const int resolved = ResolveRealMythicAttach(weaponId, attachType);
    if (resolved > 0) {
        gRuntimeAttachResolveCache[key] = resolved;
        return resolved;
    }

    const int fallback = gActiveMythicAttachFallback[attachType];
    gRuntimeAttachResolveCache[key] = fallback;
    return fallback;
}

inline bool IsWeaponItemId(uint itemId) {
    return itemId >= 10000000 && itemId < 20000000;
}

inline int ActiveAttachForContext(uint weaponId, int attachType) {
    const int selected = ActiveMythicAttachFor(weaponId, attachType);
    if (selected > 0 || gRuntimeAttachWeaponId == 0 ||
        gRuntimeAttachWeaponId == weaponId) {
        return selected;
    }
    return ActiveMythicAttachFor(gRuntimeAttachWeaponId, attachType);
}

inline uint PatchAttachMythic(void* attachItem, uint weaponId) {
    if (!AttachPtr(attachItem)) return 0;
    const uint attachId = *(uint*)((uintptr_t)attachItem + 0x10);
    const int attachType = GetAttachmentType(attachId);
    const int selected = ActiveAttachForContext(weaponId, attachType);
    const uint current = *(uint*)((uintptr_t)attachItem + 0x18);
    const uint next = selected > 0 ? (uint)selected : current;
    if (selected > 0 && current != next)
        *(uint*)((uintptr_t)attachItem + 0x18) = next;
    if (next > 0)
        gRuntimeAttachMythicById[attachId] = next;
    return next;
}

inline uint64_t LoadoutTileKey(uint weaponId, uint attachId) {
    return ((uint64_t)weaponId << 32) | attachId;
}

inline uint ActiveWeaponSkinId(int weaponId) {
    if (gActiveWeaponSkins.empty()) return 0;
    auto found = gActiveWeaponSkins.find(weaponId);
    return found != gActiveWeaponSkins.end() && found->second > 0
        ? (uint)found->second : 0;
}

inline void IndexWeaponParts() {
    if (gWeaponPartAssetInstances.size() == gPartConfIndexed) return;
    std::lock_guard<std::mutex> lock(gMythicCollectorMutex);
    for (; gPartConfIndexed < gWeaponPartAssetInstances.size(); ++gPartConfIndexed) {
        void* conf = gWeaponPartAssetInstances[gPartConfIndexed];
        if (!AttachPtr(conf)) continue;
        const int partId = *(int*)((uintptr_t)conf + 0x58);
        const uint weaponId = *(uint*)((uintptr_t)conf + 0x7C);
        if (weaponId > 0 && partId > 0) {
            const uint64_t key = ((uint64_t)weaponId << 32) | (uint32_t)partId;
            gWeaponPartConfCache[key] = conf;
        }
    }
}

inline void* FindWeaponPartConf(uint weaponId, int partId) {
    if (weaponId == 0 || partId <= 0) return nullptr;
    IndexWeaponParts();
    const uint64_t key = ((uint64_t)weaponId << 32) | (uint32_t)partId;
    auto found = gWeaponPartConfCache.find(key);
    return found != gWeaponPartConfCache.end() ? found->second : nullptr;
}

inline uint ActiveWeaponPartSkinId(int weaponId, int partId) {
    if (partId <= 0) return 0;
    uint skinId = ActiveWeaponSkinId(weaponId);
    if (skinId > 0 && FindWeaponPartConf(skinId, partId) != nullptr) return skinId;
    if (gRuntimeAttachSkinId > 0 &&
        FindWeaponPartConf(gRuntimeAttachSkinId, partId) != nullptr) {
        return gRuntimeAttachSkinId;
    }
    return 0;
}
inline void* (*orig_Weapon_InitPart)(
    void* instance, void* partInfo, bool autoActivate, bool independent) = nullptr;

inline void (*orig_AttachmentGroupNew_Ctor)(
    void* instance, uint attachId, int attachSkinId, uint mythicSkinId,
    uint weaponId, int ownState, bool enableReddot, bool isAttachTypeTrans,
    bool isPreConditionSatisfied) = nullptr;

inline void (*orig_AttachmentGroupNew_ItemCtor)(
    void* instance, void* attachItem, uint weaponId, int ownState,
    bool enableReddot, bool isAttachTypeTrans, bool isPreConditionSatisfied) = nullptr;

inline int (*orig_AttachmentGroupNew_GetColorId)(void* instance) = nullptr;

inline void (*orig_BRWeaponAttachItemData_Ctor)(
    void* instance, uint weaponActorId, uint attachId, int index,
    uint mythicId, int attachSkinId, bool disable, uint dmzItemId) = nullptr;

inline void (*orig_AttachItem_Ctor)(
    void* instance, uint itemId, int skinId, uint mythicSkinId) = nullptr;

inline void (*orig_AttachItem_SetData)(
    void* instance, uint itemId, int skinId, uint mythicSkinId) = nullptr;

inline void (*orig_AttachItem_SetMythicSkinID)(
    void* instance, int skinId, uint mythicSkinId) = nullptr;

inline void (*orig_AttachItem_CopyEquipped)(void* instance, void* item) = nullptr;

inline uint (*orig_AttachItem_GetModelAttachId)(void* instance) = nullptr;

inline void (*orig_AttachSkinData_Ctor)(
    void* instance, uint blueprintWeaponId, int skinId, uint mythicSkinId) = nullptr;

inline void (*orig_LoadoutAttachItem_SetData)(
    void* instance, void* attachInfo, void* panel) = nullptr;

inline void* hook_Weapon_InitPart(
    void* instance, void* partInfo, bool autoActivate, bool independent)
{
    const int rawWeaponId = AttachPtr(instance)
        ? *(int*)((uintptr_t)instance + 0x1030) : 0;

    int getterWeaponId = 0;
    if (AttachPtr(instance) && Weapon_GetWeaponId) {
        using GetWeaponIdFn = int (*)(void*);
        getterWeaponId = ((GetWeaponIdFn)Weapon_GetWeaponId)(instance);
    }

    uint oldSkin = 0;
    int partId = 0;
    uint selectedSkin = 0;
    if (AttachPtr(partInfo)) {
        partId = *(int*)((uintptr_t)partInfo + 0x20);
        oldSkin = *(uint*)((uintptr_t)partInfo + 0x28);
        selectedSkin = ActiveWeaponPartSkinId(rawWeaponId, partId);
        if (selectedSkin == 0 && getterWeaponId != rawWeaponId)
            selectedSkin = ActiveWeaponPartSkinId(getterWeaponId, partId);
        if (selectedSkin > 0)
            *(uint*)((uintptr_t)partInfo + 0x28) = selectedSkin;
    }

    void* part = orig_Weapon_InitPart
        ? orig_Weapon_InitPart(instance, partInfo, autoActivate, independent)
        : nullptr;

    if (selectedSkin > 0 && AttachPtr(partInfo))
        *(uint*)((uintptr_t)partInfo + 0x28) = oldSkin;

    if (selectedSkin > 0 && AttachPtr(part)) {
        const uintptr_t addr = (uintptr_t)part;
        if (*(uint*)(addr + 0x44) != selectedSkin)
            *(uint*)(addr + 0x44) = selectedSkin;
    }
    return part;
}

inline void hook_AttachmentGroupNew_Ctor(
    void* instance, uint attachId, int attachSkinId, uint mythicSkinId,
    uint weaponId, int ownState, bool enableReddot, bool isAttachTypeTrans,
    bool isPreConditionSatisfied)
{
    const int attachType = GetAttachmentType(attachId);
    if (IsWeaponItemId(weaponId)) gRuntimeAttachWeaponId = weaponId;

    const int selected = ActiveAttachForContext(weaponId, attachType);
    const uint next = selected > 0 ? (uint)selected : mythicSkinId;

    if (orig_AttachmentGroupNew_Ctor) {
        orig_AttachmentGroupNew_Ctor(instance, attachId, attachSkinId, next,
            weaponId, ownState, enableReddot, isAttachTypeTrans,
            isPreConditionSatisfied);
    }

    if (selected > 0 && AttachPtr(instance)) {
        void* item = *(void**)((uintptr_t)instance + 0x10);
        if (AttachPtr(item) && *(uint*)((uintptr_t)item + 0x18) != (uint)selected)
            *(uint*)((uintptr_t)item + 0x18) = (uint)selected;
    }
}

inline void hook_AttachmentGroupNew_ItemCtor(
    void* instance, void* attachItem, uint weaponId, int ownState,
    bool enableReddot, bool isAttachTypeTrans, bool isPreConditionSatisfied)
{
    if (IsWeaponItemId(weaponId)) gRuntimeAttachWeaponId = weaponId;
    PatchAttachMythic(attachItem, weaponId);

    if (orig_AttachmentGroupNew_ItemCtor) {
        orig_AttachmentGroupNew_ItemCtor(instance, attachItem, weaponId,
            ownState, enableReddot, isAttachTypeTrans, isPreConditionSatisfied);
    }

    if (AttachPtr(instance)) {
        void* item = *(void**)((uintptr_t)instance + 0x10);
        PatchAttachMythic(item, weaponId);
    }
}

inline int hook_AttachmentGroupNew_GetColorId(void* instance) {
    const int colorId = orig_AttachmentGroupNew_GetColorId
        ? orig_AttachmentGroupNew_GetColorId(instance) : 0;
    if (!AttachPtr(instance)) return colorId;

    void* item = *(void**)((uintptr_t)instance + 0x10);
    if (!AttachPtr(item)) return colorId;

    const uint attachId = *(uint*)((uintptr_t)item + 0x10);
    const uint weaponId = *(uint*)((uintptr_t)instance + 0x1C);
    auto preview = gLoadoutPreviewTiles.find(LoadoutTileKey(weaponId, attachId));
    const bool hasPreview = preview != gLoadoutPreviewTiles.end() && preview->second;

    if (hasPreview) return 5;
    if (colorId == 5) {
        void* conf = *(void**)((uintptr_t)instance + 0x30);
        return AttachPtr(conf) ? *(uint8_t*)((uintptr_t)conf + 0x41) : 0;
    }
    return colorId;
}

inline void hook_LoadoutAttachItem_SetData(
    void* instance, void* attachInfo, void* panel)
{
    uint weaponId = 0, attachId = 0, uiMythic = 0, previewSkin = 0;
    int attachType = 0;
    void* item = nullptr;
    void* previewConf = nullptr;

    if (AttachPtr(attachInfo)) {
        weaponId = *(uint*)((uintptr_t)attachInfo + 0x1C);
        item = *(void**)((uintptr_t)attachInfo + 0x10);
        if (AttachPtr(item)) {
            attachId = *(uint*)((uintptr_t)item + 0x10);
            attachType = GetAttachmentType(attachId);
        }
        const uint activeSkin = ActiveWeaponSkinId((int)weaponId);
        previewConf = FindWeaponPartConf(activeSkin, (int)attachId);
        previewSkin = AttachPtr(previewConf) ? activeSkin : 0;
        if (previewSkin > 0) {
            const int selected = ActiveAttachForContext(weaponId, attachType);
            uiMythic = IsRealMythicAttachSkin((uint)selected)
                ? (uint)selected : previewSkin;
            gLoadoutPreviewTiles.emplace(LoadoutTileKey(weaponId, attachId), true);
        } else {
            gLoadoutPreviewTiles.erase(LoadoutTileKey(weaponId, attachId));
        }
        if (AttachPtr(item) && *(uint*)((uintptr_t)item + 0x18) != uiMythic)
            *(uint*)((uintptr_t)item + 0x18) = uiMythic;
        if (AttachPtr(previewConf) &&
            *(void**)((uintptr_t)attachInfo + 0x28) != previewConf) {
            *(void**)((uintptr_t)attachInfo + 0x28) = previewConf;
        }
    }

    if (orig_LoadoutAttachItem_SetData)
        orig_LoadoutAttachItem_SetData(instance, attachInfo, panel);

    if (AttachPtr(attachInfo) && AttachPtr(previewConf) &&
        *(void**)((uintptr_t)attachInfo + 0x28) != previewConf) {
        *(void**)((uintptr_t)attachInfo + 0x28) = previewConf;
    }
    if (AttachPtr(instance) && *(uint*)((uintptr_t)instance + 0xA0) != uiMythic)
        *(uint*)((uintptr_t)instance + 0xA0) = uiMythic;

    if (!AttachPtr(attachInfo) || uiMythic == 0) return;
    void* part = *(void**)((uintptr_t)attachInfo + 0x28);
    if (!AttachPtr(item) || !AttachPtr(part)) return;

    const uint partItemId = *(uint*)((uintptr_t)part + 0x58);
    if (attachType <= 0 || attachType >= 31 || !IsRealMythicAttachSkin(partItemId))
        return;

    gActiveMythicAttachments[(int)weaponId][attachType] = (int)partItemId;
    gRuntimeAttachMythicById[attachId] = partItemId;
    if (*(uint*)((uintptr_t)item + 0x18) != partItemId)
        *(uint*)((uintptr_t)item + 0x18) = partItemId;
    if (AttachPtr(instance) && *(uint*)((uintptr_t)instance + 0xA0) != partItemId)
        *(uint*)((uintptr_t)instance + 0xA0) = partItemId;
}

inline void hook_BRWeaponAttachItemData_Ctor(
    void* instance, uint weaponActorId, uint attachId, int index,
    uint mythicId, int attachSkinId, bool disable, uint dmzItemId)
{
    const int attachType = GetAttachmentType(attachId);
    if (IsWeaponItemId(weaponActorId)) gRuntimeAttachWeaponId = weaponActorId;
    const int selected = ActiveAttachForContext(weaponActorId, attachType);
    const uint next = selected > 0 ? (uint)selected : mythicId;

    if (orig_BRWeaponAttachItemData_Ctor) {
        orig_BRWeaponAttachItemData_Ctor(instance, weaponActorId, attachId,
            index, next, attachSkinId, disable, dmzItemId);
    }

    if (selected > 0 && AttachPtr(instance))
        *(uint*)((uintptr_t)instance + 0x14) = (uint)selected;
}

inline void hook_AttachItem_Ctor(
    void* instance, uint itemId, int skinId, uint mythicSkinId)
{
    const int attachType = GetAttachmentType(itemId);
    if (attachType == 0 && IsWeaponItemId(itemId)) gRuntimeAttachWeaponId = itemId;
    const int selected = ActiveAttachForContext(gRuntimeAttachWeaponId, attachType);
    auto cached = gRuntimeAttachMythicById.find(itemId);
    const uint next = selected > 0
        ? (uint)selected
        : (mythicSkinId > 0
            ? mythicSkinId
            : (cached != gRuntimeAttachMythicById.end() ? cached->second : 0));

    if (orig_AttachItem_Ctor) orig_AttachItem_Ctor(instance, itemId, skinId, next);

    if (next > 0) {
        gRuntimeAttachMythicById[itemId] = next;
        if (AttachPtr(instance) && *(uint*)((uintptr_t)instance + 0x18) != next)
            *(uint*)((uintptr_t)instance + 0x18) = next;
    }
}

inline void hook_AttachItem_SetData(
    void* instance, uint itemId, int skinId, uint mythicSkinId)
{
    const int attachType = GetAttachmentType(itemId);
    if (attachType == 0 && IsWeaponItemId(itemId)) gRuntimeAttachWeaponId = itemId;
    const int selected = ActiveAttachForContext(gRuntimeAttachWeaponId, attachType);
    auto cached = gRuntimeAttachMythicById.find(itemId);
    const uint next = selected > 0
        ? (uint)selected
        : (mythicSkinId > 0
            ? mythicSkinId
            : (cached != gRuntimeAttachMythicById.end() ? cached->second : 0));

    if (orig_AttachItem_SetData) orig_AttachItem_SetData(instance, itemId, skinId, next);

    if (next > 0) {
        gRuntimeAttachMythicById[itemId] = next;
        if (AttachPtr(instance) && *(uint*)((uintptr_t)instance + 0x18) != next)
            *(uint*)((uintptr_t)instance + 0x18) = next;
    }
}

inline void hook_AttachItem_SetMythicSkinID(
    void* instance, int skinId, uint mythicSkinId)
{
    uint itemId = 0;
    if (AttachPtr(instance)) itemId = *(uint*)((uintptr_t)instance + 0x10);
    const int attachType = GetAttachmentType(itemId);
    if (attachType == 0 && IsWeaponItemId(itemId)) gRuntimeAttachWeaponId = itemId;
    const int selected = ActiveAttachForContext(gRuntimeAttachWeaponId, attachType);
    auto cached = gRuntimeAttachMythicById.find(itemId);
    const uint next = selected > 0
        ? (uint)selected
        : (mythicSkinId > 0
            ? mythicSkinId
            : (cached != gRuntimeAttachMythicById.end() ? cached->second : 0));

    if (orig_AttachItem_SetMythicSkinID)
        orig_AttachItem_SetMythicSkinID(instance, skinId, next);

    if (next > 0) {
        gRuntimeAttachMythicById[itemId] = next;
        if (AttachPtr(instance) && *(uint*)((uintptr_t)instance + 0x18) != next)
            *(uint*)((uintptr_t)instance + 0x18) = next;
    }
}

inline void hook_AttachItem_CopyEquipped(void* instance, void* item) {
    uint attachId = 0;
    if (AttachPtr(item)) attachId = *(uint*)((uintptr_t)item + 0x3C);
    const int attachType = GetAttachmentType(attachId);
    const int selected = ActiveAttachForContext(gRuntimeAttachWeaponId, attachType);

    if (orig_AttachItem_CopyEquipped) orig_AttachItem_CopyEquipped(instance, item);

    if (AttachPtr(instance) && IsRealMythicAttachSkin((uint)selected))
        *(uint*)((uintptr_t)instance + 0x18) = (uint)selected;
}

inline uint hook_AttachItem_GetModelAttachId(void* instance) {
    if (AttachPtr(instance)) {
        const uint attachId = *(uint*)((uintptr_t)instance + 0x10);
        const int attachType = GetAttachmentType(attachId);
        const int selected = ActiveAttachForContext(gRuntimeAttachWeaponId, attachType);
        if (IsRealMythicAttachSkin((uint)selected)) return (uint)selected;
    }
    return orig_AttachItem_GetModelAttachId
        ? orig_AttachItem_GetModelAttachId(instance) : 0;
}

inline void hook_AttachSkinData_Ctor(
    void* instance, uint blueprintWeaponId, int skinId, uint mythicSkinId)
{
    const int attachType = GetAttachmentType((uint)skinId);
    if (IsWeaponItemId(blueprintWeaponId)) gRuntimeAttachWeaponId = blueprintWeaponId;
    const int selected = ActiveAttachForContext(blueprintWeaponId, attachType);
    const uint next = selected > 0 ? (uint)selected : mythicSkinId;

    if (orig_AttachSkinData_Ctor)
        orig_AttachSkinData_Ctor(instance, blueprintWeaponId, skinId, next);
}

inline std::array<int, 31> BuildMythicAttach(int confId, int baseId, bool isMythic) {
    std::array<int, 31> result{};
    for (void* weaponSkin : WeaponSkinConfigRef()) {
        if (!AttachPtr(weaponSkin)) continue;
        auto* fields = (WeaponSkinFields*)((uintptr_t)weaponSkin + 0x10);
        const bool match =
            (int)fields->ID == confId ||
            (int)fields->WeaponID == baseId ||
            (int)fields->BRWeaponID == baseId ||
            fields->BrSkinWeaponID == baseId;
        if (!match) continue;
        for (int attachType : MythicAttachTypes()) {
            if (attachType <= 0 || attachType >= (int)result.size()) continue;
            const int mythicId = RealMythicForType(fields, attachType);
            if (mythicId > 0) result[attachType] = mythicId;
        }
    }
    if (isMythic && confId > 0) {
        static const std::array<int, 8> previewTypes = {1, 4, 5, 6, 8, 9, 10, 12};
        for (int attachType : previewTypes) {
            if (attachType > 0 && attachType < (int)result.size() &&
                result[attachType] <= 0) {
                result[attachType] = confId;
            }
        }
    }
    return result;
}

struct AttachWeaponIds {
    int baseId, extraId, skinId, itemId, lootId;
};

inline void ApplyMythicAttachments(
    const AttachWeaponIds& ids,
    const std::array<int, 31>& mythicAttach)
{
    gActiveWeaponSkins.clear();
    gActiveMythicAttachments.clear();
    gRuntimeAttachMythicById.clear();
    gRuntimeAttachResolveCache.clear();
    gLoadoutPreviewTiles.clear();
    gRuntimeAttachWeaponId = 0;
    gRuntimeAttachSkinId = ids.skinId > 0 ? (uint)ids.skinId : 0;

    const std::array<int, 5> keys = {
        ids.baseId, ids.extraId, ids.skinId, ids.itemId, ids.lootId
    };
    for (int key : keys) {
        if (key <= 0) continue;
        gActiveMythicAttachments[key] = mythicAttach;
        if (ids.skinId > 0) gActiveWeaponSkins[key] = ids.skinId;
    }

    for (void* weaponSkin : WeaponSkinConfigRef()) {
        if (!AttachPtr(weaponSkin)) continue;
        auto* fields = (WeaponSkinFields*)((uintptr_t)weaponSkin + 0x10);
        const bool selected =
            (ids.skinId > 0 && (int)fields->ID == ids.skinId) ||
            (ids.baseId > 0 && ((int)fields->WeaponID == ids.baseId ||
                                (int)fields->BRWeaponID == ids.baseId ||
                                fields->BrSkinWeaponID == ids.baseId));
        if (!selected) continue;
        const int aliases[] = {(int)fields->ID, (int)fields->WeaponID,
                               (int)fields->BRWeaponID, fields->BrSkinWeaponID};
        for (int alias : aliases) {
            if (alias <= 0) continue;
            gActiveMythicAttachments[alias] = mythicAttach;
            if (ids.skinId > 0) gActiveWeaponSkins[alias] = ids.skinId;
        }
    }
    gActiveMythicAttachFallback = mythicAttach;
}

static std::vector<int> SelectedAliases(const itemInfo& skin) {
    std::vector<int> ids;
    auto add = [&](int v) {
        if (v <= 0) return;
        if (std::find(ids.begin(), ids.end(), v) == ids.end()) ids.push_back(v);
    };
    add(skin.WeaponConf[0]);
    add(skin.WeaponExtra[0]);
    add(skin.WeaponConf[2]);
    add(skin.Item2Inventory[0]);
    add(skin.Item2Inventory[3]);

    for (void* ws : WeaponSkinConfigRef()) {
        if (!AttachPtr(ws)) continue;
        auto* f = (WeaponSkinFields*)((uintptr_t)ws + 0x10);
        const bool match = (int)f->ID == skin.WeaponConf[2] ||
                           (int)f->WeaponID == skin.WeaponConf[0] ||
                           (int)f->BRWeaponID == skin.WeaponConf[0] ||
                           f->BrSkinWeaponID == skin.WeaponConf[0];
        if (!match) continue;
        add((int)f->ID);
        add((int)f->WeaponID);
        add((int)f->BRWeaponID);
        add(f->BrSkinWeaponID);
    }
    return ids;
}

static void MapRuntimeValue(std::unordered_map<int, int>& map,
                            const std::vector<int>& ids, int value) {
    if (value <= 0) return;
    for (int id : ids) if (id > 0) map[id] = value;
}

static bool BuildDatabase() {
    const Snapshot& s = GetSnapshotRef();
    if (s.conf.empty() || s.inventory.empty()) return false;

    std::unordered_map<int, void*> inventoryById, extraById, assetById,
                                   fireById, resourceById;
    std::unordered_map<int, int> armorByIcon, sightByIcon, killByIcon;

    inventoryById.reserve(s.inventory.size());
    extraById.reserve(s.extra.size());
    assetById.reserve(s.asset.size());
    fireById.reserve(s.fire.size());
    resourceById.reserve(s.resource.size());

    for (void* v : s.inventory) {
        if (!Valid(v)) continue;
        auto* f = (Item2InventoryFields*)((uintptr_t)v + 0x20);
        if (Valid(f) && f->ItemID > 0) inventoryById[f->ItemID] = v;
    }
    for (void* v : s.extra) {
        if (!Valid(v)) continue;
        auto* f = (WeaponConfExtraFields*)((uintptr_t)v + 0x10);
        if (Valid(f) && f->ID > 0) extraById[(int)f->ID] = v;
    }
    for (void* v : s.asset) {
        if (!Valid(v)) continue;
        auto* f = (WeaponAssetGroupFields*)((uintptr_t)v + 0x40);
        if (Valid(f) && f->Id > 0) assetById[f->Id] = v;
    }
    for (void* v : s.fire) {
        if (!Valid(v)) continue;
        const int id = *(int*)((uintptr_t)v + 0x80);
        if (id > 0) fireById[id] = v;
    }
    for (void* v : s.resource) {
        if (!Valid(v)) continue;
        auto* f = (ItemResourceFields*)((uintptr_t)v + 0x10);
        if (Valid(f) && f->ID > 0) resourceById[f->ID] = v;
    }
    for (void* v : s.armor) {
        if (!Valid(v)) continue;
        auto* f = (MythicArmorFields*)((uintptr_t)v + 0x14);
        if (Valid(f) && f->SecondTab > 0 && f->ThirdTab == 5)
            armorByIcon[f->SecondTab] = f->AssetID;
    }
    for (void* v : s.sight) {
        if (!Valid(v)) continue;
        Array<int>* equip = *(Array<int>**)((uintptr_t)v + 0x38);
        const int icon = ArrayLogicalLast(equip);
        if (icon > 0) sightByIcon[icon] = *(int*)((uintptr_t)v + 0x14);
    }
    for (void* v : s.kill) {
        if (!Valid(v)) continue;
        Array<int>* equip = *(Array<int>**)((uintptr_t)v + 0x90);
        Array<int>* effects = *(Array<int>**)((uintptr_t)v + 0x10);
        const int icon = ArrayLogicalLast(equip);
        const int effect = ArrayLogicalLast(effects);
        if (icon > 0 && effect > 0) killByIcon[icon] = effect;
    }

    std::vector<itemInfo> built;
    built.reserve(s.conf.size());
    std::unordered_map<std::string, int> duplicates;
    std::unordered_set<int> seenConfIds;
    seenConfIds.reserve(s.conf.size());

    for (void* conf : s.conf) {
        if (!Valid(conf)) continue;
        auto* cf = (WeaponConfFields*)((uintptr_t)conf + 0x20);
        if (!Valid(cf)) continue;
        const int baseId = (int)cf->BaseWeaponId;
        const int confId = (int)cf->ID;
        if (baseId <= 0 || confId <= 0) continue;
        auto ii = inventoryById.find(confId);
        if (ii == inventoryById.end()) continue;
        auto* inv = (Item2InventoryFields*)((uintptr_t)ii->second + 0x20);
        if (!Valid(inv)) continue;

        std::string realName = GetNameString(kWeaponNameGetter, conf);
        if (realName.empty()) continue;
        if (!seenConfIds.insert(confId).second) continue;
        std::string tagged = std::string(RarityPrefix((int)cf->ColorID)) + realName;
        int& dup = duplicates[tagged];
        if (dup++ > 0) tagged += " +" + std::to_string(dup - 1);

        const int assetGroupId = inv->WeaponAssetGroupID;
        const int weaponIconId = inv->WeaponIconID;
        int originalFireId = 0;
        int selectedFireId = 0;
        std::array<int, 4> weaponAttach{};

        auto ai = assetById.find(assetGroupId);
        if (ai != assetById.end()) {
            auto* af = (WeaponAssetGroupFields*)((uintptr_t)ai->second + 0x40);
            if (Valid(af)) { originalFireId = af->FireEffectGroupID; selectedFireId = originalFireId; }
        }

        if (selectedFireId > 0) {
            auto fi = fireById.find(selectedFireId);
            if (cf->ColorID == 5 && fi != fireById.end()) {
                const int smoke = *(int*)((uintptr_t)fi->second + 0x1C);
                if (smoke == 0) {
                    for (int d = 1; d <= 10; ++d) {
                        auto cand = fireById.find(originalFireId + d);
                        if (cand == fireById.end()) continue;
                        if (*(int*)((uintptr_t)cand->second + 0x1C) != 0) {
                            selectedFireId = originalFireId + d;
                            break;
                        }
                    }
                }
            }
            auto sel = fireById.find(selectedFireId);
            if (sel != fireById.end()) {
                const uintptr_t f = (uintptr_t)sel->second;
                weaponAttach = {
                    *(int*)(f + 0x64), *(int*)(f + 0x68),
                    *(int*)(f + 0x6C), *(int*)(f + 0x70)
                };
            }
        }

        int extraAlias = 0, mythicArmor = 0, mythicSight = 0,
            deadReplay = 0, killEffect = 0;
        if (extraById.find(baseId) != extraById.end()) extraAlias = baseId;
        auto se = extraById.find(confId);
        if (se != extraById.end()) {
            auto* ef = (WeaponConfExtraFields*)((uintptr_t)se->second + 0x10);
            if (Valid(ef)) {
                deadReplay = ef->DefaultDeadReplayEffectId;
                killEffect = ef->DefaultKillEffectId;
            }
        }
        if (cf->ColorID == 5) {
            auto a = armorByIcon.find(weaponIconId);
            if (a != armorByIcon.end()) mythicArmor = a->second;
            auto si = sightByIcon.find(weaponIconId);
            if (si != sightByIcon.end()) mythicSight = si->second;
            auto ki = killByIcon.find(weaponIconId);
            if (ki != killByIcon.end()) killEffect = ki->second;
        }

        int fx = 0, invModel = 0, rawModel = 0;
        void *mini = nullptr, *small = nullptr, *sprite = nullptr, *square = nullptr;
        auto ri = resourceById.find(confId);
        if (ri != resourceById.end()) {
            auto* rf = (ItemResourceFields*)((uintptr_t)ri->second + 0x10);
            if (Valid(rf)) {
                fx = rf->FxAssetID; invModel = rf->InventoryModelID;
                rawModel = rf->ModelAssetIDRaw;
                mini = rf->UIMiniSpriteName; small = rf->UISmallSpriteName;
                sprite = rf->UISpriteName; square = rf->UISquareSpriteName;
            }
        }

        const bool isMythic = cf->ColorID == 5;
        const std::array<int, 31> mythicAttach = BuildMythicAttach(confId, baseId, isMythic);

        built.push_back({
            tagged,
            {baseId, assetGroupId, weaponIconId, baseId + 200},
            {baseId, (int)cf->ColorID, confId, cf->DefaultKillBrocast, cf->DefWeaponSkinID},
            {extraAlias, mythicArmor, mythicSight, deadReplay, killEffect},
            {assetGroupId, selectedFireId, originalFireId},
            weaponAttach,
            mythicAttach,
            {fx, invModel, rawModel},
            {mini, small, sprite, square}
        });
    }

    std::sort(built.begin(), built.end(), [](const itemInfo& a, const itemInfo& b) {
        return ToLower(a.itemName) < ToLower(b.itemName);
    });

    const bool ready = !built.empty();
    {
        std::lock_guard<std::mutex> lock(gDatabaseMutex);
        itemData.swap(built);
        nameCountMap.clear();
    }
    gDbVersion.fetch_add(1, std::memory_order_release);
    gDatabaseReady.store(ready, std::memory_order_release);
    return gDatabaseReady.load(std::memory_order_acquire);
}

static void ApplyToCurrentInstances(const itemInfo& skin) {
    const Snapshot& s = GetSnapshotRef();
    const int baseId = skin.WeaponConf[0];
    const int extraId = skin.WeaponExtra[0];
    const int skinId = skin.WeaponConf[2];
    const int itemId = skin.Item2Inventory[0];
    const int lootId = skin.Item2Inventory[3];
    const std::vector<int> aliases = SelectedAliases(skin);
    auto matches = [&](int id) {
        return std::find(aliases.begin(), aliases.end(), id) != aliases.end();
    };

    for (void* v : s.inventory) {
        if (!Valid(v)) continue;
        auto* f = (Item2InventoryFields*)((uintptr_t)v + 0x20);
        if (!Valid(f)) continue;
        if (f->ItemID == itemId || f->ItemID == lootId || f->ItemID == baseId) {
            f->WeaponAssetGroupID = skin.Item2Inventory[1];
            f->WeaponIconID = skin.Item2Inventory[2];
        }
    }

    void* selectedName = nullptr;
    for (void* v : s.conf) {
        if (!Valid(v)) continue;
        auto* f = (WeaponConfFields*)((uintptr_t)v + 0x20);
        if (Valid(f) && (int)f->ID == skinId) { selectedName = f->LOCID_Name; break; }
    }
    for (void* v : s.conf) {
        if (!Valid(v)) continue;
        auto* f = (WeaponConfFields*)((uintptr_t)v + 0x20);
        if (!Valid(f) || !matches((int)f->ID)) continue;
        f->ColorID = (uint8_t)skin.WeaponConf[1];
        f->DefWeaponSkinID = skinId;
        f->DefaultKillBrocast = skin.WeaponConf[3];
        if (selectedName) f->LOCID_Name = selectedName;
    }

    for (void* v : s.extra) {
        if (!Valid(v)) continue;
        auto* f = (WeaponConfExtraFields*)((uintptr_t)v + 0x10);
        if (!Valid(f) || !matches((int)f->ID)) continue;
        if (skin.WeaponConf[1] == 5) {
            f->DefaultMythicArmor = (uint8_t)skin.WeaponExtra[1];
            f->DefaultMythicSig = (uint8_t)skin.WeaponExtra[2];
        }
        f->DefaultDeadReplayEffectId = skin.WeaponExtra[3];
        f->DefaultKillEffectId = skin.WeaponExtra[4];
    }

    for (void* v : s.asset) {
        if (!Valid(v)) continue;
        auto* f = (WeaponAssetGroupFields*)((uintptr_t)v + 0x40);
        if (Valid(f) && f->Id == skin.WeaponAsset[0] && skin.WeaponAsset[1] > 0)
            f->FireEffectGroupID = skin.WeaponAsset[1];
    }

    if (skin.WeaponAttach[0] || skin.WeaponAttach[1] ||
        skin.WeaponAttach[2] || skin.WeaponAttach[3]) {
        for (void* v : s.fire) {
            if (!Valid(v)) continue;
            const uintptr_t f = (uintptr_t)v;
            const int fireId = *(int*)(f + 0x80);
            if (fireId != skin.WeaponAsset[1] && fireId != skin.WeaponAsset[2]) continue;
            *(int*)(f + 0x64) = skin.WeaponAttach[0];
            *(int*)(f + 0x68) = skin.WeaponAttach[1];
            *(int*)(f + 0x6C) = skin.WeaponAttach[2];
            *(int*)(f + 0x70) = skin.WeaponAttach[3];
        }
    }

    for (void* v : s.resource) {
        if (!Valid(v)) continue;
        auto* f = (ItemResourceFields*)((uintptr_t)v + 0x10);
        if (!Valid(f) || !matches(f->ID)) continue;
        f->FxAssetID = skin.ItemResInt[0];
        f->InventoryModelID = skin.ItemResInt[1];
        f->ModelAssetIDRaw = skin.ItemResInt[2];
        f->UIMiniSpriteName = skin.ItemRes[0];
        f->UISmallSpriteName = skin.ItemRes[1];
        f->UISpriteName = skin.ItemRes[2];
        f->UISquareSpriteName = skin.ItemRes[3];
    }

    {
        std::lock_guard<std::mutex> lock(gEffectMutex);
        gActiveKillEffects.clear();
        gActiveFireEffects.clear();
        gActiveBulletEffects.clear();
        gActiveBroadcastEffects.clear();
        MapRuntimeValue(gActiveKillEffects, aliases, skin.WeaponExtra[4]);
        MapRuntimeValue(gActiveFireEffects, aliases, skin.WeaponAsset[1]);
        MapRuntimeValue(gActiveBulletEffects, aliases, skin.WeaponAsset[1]);
        MapRuntimeValue(gActiveBroadcastEffects, aliases, skin.WeaponConf[3]);
    }

    ApplyMythicAttachments({baseId, extraId, skinId, itemId, lootId}, skin.MythicAttach);
}

static void SelectSkin(const itemInfo& skin) {
    {
        std::lock_guard<std::mutex> lock(gSelectionMutex);
        gSelectedSkin = skin;
        gSelectedName = skin.itemName;
        gHasSelectedSkin = true;
    }
    ApplyToCurrentInstances(skin);
}

static int CurrentWeaponId() {
    Pawn* pawn = GamePlay::get_LocalPawn();
    if (!pawn) return 0;
    Weapon* weapon = pawn->get_CurrentWeapon();
    return weapon ? weapon->get_WeaponID() : 0;
}

static int ResolveEffect(void* instance, EffectGetter original,
                         const std::unordered_map<int, int>& values) {
    if (values.empty()) return original ? original(instance) : 0;
    const int id = CurrentWeaponId();
    std::lock_guard<std::mutex> lock(gEffectMutex);
    auto it = values.find(id);
    if (it != values.end() && it->second > 0) return it->second;
    return original ? original(instance) : 0;
}

static int HookKillEffect(void* p) { return ResolveEffect(p, gOrigKillEffect, gActiveKillEffects); }
static int HookFireEffect(void* p) { return ResolveEffect(p, gOrigFireEffect, gActiveFireEffects); }
static int HookBulletEffect(void* p) { return ResolveEffect(p, gOrigBulletTrack, gActiveBulletEffects); }
static int HookBroadcast(void* p) { return ResolveEffect(p, gOrigBroadcast, gActiveBroadcastEffects); }

static bool InstallOne(uintptr_t base, uintptr_t rva, void* hook, void** orig) {
    void* target = (void*)(base + rva);
    if (!Valid(target)) return false;
    DobbyHook(target, hook, orig);
    return true;
}

static void Worker() {
    size_t previousFingerprint = 0;
    size_t lastBuiltFingerprint = 0;
    int stablePasses = 0;

    while (gInstalled.load(std::memory_order_acquire)) {
        if (!gScanRequested.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        const size_t current = Fingerprint();
        if (current == 0) { stablePasses = 0; previousFingerprint = 0; continue; }
        if (current == previousFingerprint) ++stablePasses;
        else stablePasses = 0;
        previousFingerprint = current;

        if (stablePasses < 1 || current == lastBuiltFingerprint) continue;
        if (BuildDatabase()) lastBuiltFingerprint = current;

        itemInfo selected;
        bool have = false;
        {
            std::lock_guard<std::mutex> lock(gSelectionMutex);
            have = gHasSelectedSkin;
            if (have) selected = gSelectedSkin;
        }
        if (have) ApplyToCurrentInstances(selected);
    }
}

inline bool InstallCaptureOnly() {
    if (gInstalled.exchange(true)) return true;
    const uintptr_t base = Tools::GetBaseAddress("libunity.so");
    if (!base) { gInstalled.store(false); return false; }

    InstallOne(base, kItemInventoryCtor,   (void*)HookInventory,      (void**)&gOrigInventory);
    InstallOne(base, kWeaponConfCtor,      (void*)HookConf,           (void**)&gOrigConf);
    InstallOne(base, kWeaponExtraCtor,     (void*)HookExtra,          (void**)&gOrigExtra);
    InstallOne(base, kWeaponFireCtor,      (void*)HookFire,           (void**)&gOrigFire);
    InstallOne(base, kWeaponAssetCtor,     (void*)HookAsset,          (void**)&gOrigAsset);
    InstallOne(base, kMythicArmorCtor,     (void*)HookArmor,          (void**)&gOrigArmor);
    InstallOne(base, kMythicSightCtor,     (void*)HookSight,          (void**)&gOrigSight);
    InstallOne(base, kKillEffectCtor,      (void*)HookKill,           (void**)&gOrigKill);
    InstallOne(base, kWeaponSkinCtor,      (void*)HookWeaponSkin,     (void**)&gOrigWeaponSkin);
    InstallOne(base, kItemResourceCtor,    (void*)HookResource,       (void**)&gOrigResource);
    InstallOne(base, kWeaponPartAssetCtor, (void*)HookWeaponPartAsset,(void**)&gOrigWeaponPartAsset);
    InstallOne(base, kAttachmentConfCtor,  (void*)HookAttachmentConf, (void**)&gOrigAttachmentConf);

    if (!gWorkerStarted.exchange(true)) std::thread(Worker).detach();
    return true;
}

inline bool InstallRuntimeOnly() {
    if (gRuntimeInstalled.exchange(true)) return true;
    if (!InstallCaptureOnly()) { gRuntimeInstalled.store(false); return false; }
    const uintptr_t base = Tools::GetBaseAddress("libunity.so");
    if (!base) { gRuntimeInstalled.store(false); return false; }

    InstallOne(base, kCurrentKillEffect,  (void*)HookKillEffect,  (void**)&gOrigKillEffect);
    InstallOne(base, kCurrentFireEffect,  (void*)HookFireEffect,  (void**)&gOrigFireEffect);
    InstallOne(base, kCurrentBulletTrack, (void*)HookBulletEffect,(void**)&gOrigBulletTrack);
    InstallOne(base, kCurrentBroadcast,   (void*)HookBroadcast,   (void**)&gOrigBroadcast);

    InstallOne(base, kAttachmentGroupNewCtor,       (void*)hook_AttachmentGroupNew_Ctor,       (void**)&orig_AttachmentGroupNew_Ctor);
    InstallOne(base, kAttachmentGroupNewItemCtor,   (void*)hook_AttachmentGroupNew_ItemCtor,   (void**)&orig_AttachmentGroupNew_ItemCtor);
    InstallOne(base, kAttachmentGroupNewGetColorId, (void*)hook_AttachmentGroupNew_GetColorId, (void**)&orig_AttachmentGroupNew_GetColorId);
    InstallOne(base, kBRWeaponAttachItemDataCtor,   (void*)hook_BRWeaponAttachItemData_Ctor,   (void**)&orig_BRWeaponAttachItemData_Ctor);
    InstallOne(base, kAttachItemCtor,               (void*)hook_AttachItem_Ctor,               (void**)&orig_AttachItem_Ctor);
    InstallOne(base, kAttachItemSetData,            (void*)hook_AttachItem_SetData,            (void**)&orig_AttachItem_SetData);
    InstallOne(base, kAttachItemSetMythicSkinId,    (void*)hook_AttachItem_SetMythicSkinID,    (void**)&orig_AttachItem_SetMythicSkinID);
    InstallOne(base, kAttachItemCopyEquipped,       (void*)hook_AttachItem_CopyEquipped,       (void**)&orig_AttachItem_CopyEquipped);
    InstallOne(base, kAttachItemGetModelAttachId,   (void*)hook_AttachItem_GetModelAttachId,   (void**)&orig_AttachItem_GetModelAttachId);
    InstallOne(base, kAttachSkinDataCtor,           (void*)hook_AttachSkinData_Ctor,           (void**)&orig_AttachSkinData_Ctor);
    InstallOne(base, kLoadoutAttachItemSetData,     (void*)hook_LoadoutAttachItem_SetData,     (void**)&orig_LoadoutAttachItem_SetData);
    InstallOne(base, kWeaponInitPart,               (void*)hook_Weapon_InitPart,               (void**)&orig_Weapon_InitPart);

    return true;
}

inline bool InstallAll(uintptr_t unityBase) {
    if (!unityBase) return false;
    return InstallCaptureOnly() && InstallRuntimeOnly();
}

inline void SetScanActive(bool active) {
    gScanRequested.store(active, std::memory_order_release);
}

// ============================================================================
// DB cache + UI
// ============================================================================
inline const std::vector<itemInfo>& DatabaseRef() {
    static thread_local std::vector<itemInfo> cached;
    static thread_local uint64_t cachedVersion = 0;
    const uint64_t v = gDbVersion.load(std::memory_order_acquire);
    if (cachedVersion != v) {
        std::lock_guard<std::mutex> lock(gDatabaseMutex);
        cached = itemData;
        cachedVersion = v;
    }
    return cached;
}

inline std::vector<itemInfo> DatabaseSnapshot() {
    std::lock_guard<std::mutex> lock(gDatabaseMutex);
    return itemData;
}

inline void DrawPanel(float scale) {
    if (!gInstalled.load(std::memory_order_acquire) && !InstallCaptureOnly()) {
        ImGui::TextDisabled("SkinHax waiting for libunity.so...");
        return;
    }
    if (!gRuntimeInstalled.load(std::memory_order_acquire) && !InstallRuntimeOnly()) {
        ImGui::TextDisabled("SkinHax runtime hooks are not ready yet...");
        return;
    }
    gScanRequested.store(true, std::memory_order_release);

    const size_t uiFingerprint = Fingerprint();
    if (!gDatabaseReady.load(std::memory_order_acquire) &&
        uiFingerprint != 0 && uiFingerprint != gLastUiBuildFingerprint) {
        gLastUiBuildFingerprint = uiFingerprint;
        BuildDatabase();
    }

    const std::vector<itemInfo>& db = DatabaseRef();
    const float buttonH = std::max(36.0f, 42.0f * scale);
    ImGui::SetNextItemWidth(-145.0f * scale);
    if (ImGui::InputText("##SKINHAX_SEARCH", searchQuery, sizeof(searchQuery))) {}
    if (ImGui::IsItemClicked() || ImGui::IsItemActive()) gSearchKeyboard = true;
    ImGui::SameLine();
    if (ImGui::Button("KEYBOARD", ImVec2(130.0f * scale, buttonH))) gSearchKeyboard = !gSearchKeyboard;

    int rarityCounts[4] = {0, 0, 0, 0};
    std::string query = ToLower(std::string(searchQuery));
    std::vector<const itemInfo*> filtered;
    filtered.reserve(db.size());
    for (const auto& skin : db) {
        const int rarity = RarityFromName(skin.itemName);
        if (rarity >= 1 && rarity <= 4) ++rarityCounts[rarity - 1];
        if (rarity != gRarityFilter) continue;
        if (!query.empty()) {
            const std::string& n = skin.itemName;
            const char* body = (n.size() >= 4 && n[0] == '[' && n[2] == ']')
                                   ? n.c_str() + 4 : n.c_str();
            if (!strcasestr(body, query.c_str())) continue;
        }
        filtered.push_back(&skin);
    }

    std::array<std::string, 4> tabs = {
        "MYTHIC (" + std::to_string(rarityCounts[0]) + ")",
        "LEGENDARY (" + std::to_string(rarityCounts[1]) + ")",
        "EPIC (" + std::to_string(rarityCounts[2]) + ")",
        "COMMON (" + std::to_string(rarityCounts[3]) + ")"
    };
    const float tabW = std::max(80.0f, (ImGui::GetContentRegionAvail().x - 4.0f * 3.0f) / 4.0f);
    for (int i = 0; i < 4; ++i) {
        if (i) ImGui::SameLine(0.0f, 4.0f);
        const int rarityId = i + 1;
        if (DrawRarityTabBox(tabs[i].c_str(), rarityId, gRarityFilter == rarityId, ImVec2(tabW, buttonH)))
            gRarityFilter = rarityId;
    }

    if (db.empty())
        ImGui::TextDisabled("Loading WeaponConf / inventory skin data...");
    else
        ImGui::TextDisabled("%d named weapon skins loaded | %d shown", (int)db.size(), (int)filtered.size());
    ImGui::Separator();

    const float actionsH = 62.0f * scale;
    if (ImGui::BeginChild("##SKINHAX_LIST", ImVec2(0, -actionsH), true)) {
        for (const itemInfo* skin : filtered) {
            if (!skin) continue;
            const int rarity = RarityFromName(skin->itemName);
            const std::string& n = skin->itemName;
            const char* clean = (n.size() >= 4 && n[0] == '[' && n[2] == ']')
                                    ? n.c_str() + 4 : n.c_str();
            const bool selected = gSelectedName == skin->itemName;
            const ImVec4 rarityColor = RarityColor(rarity);
            ImGui::PushID(skin->WeaponConf[2]);
            ImGui::PushStyleColor(ImGuiCol_Header, WithAlpha(rarityColor, selected ? 0.26f : 0.10f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, WithAlpha(rarityColor, 0.20f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, WithAlpha(rarityColor, 0.30f));
            const bool rowPressed = ImGui::Selectable(
                clean, selected, 0, ImVec2(0, std::max(42.0f, 48.0f * scale)));
            const ImVec2 rowMin = ImGui::GetItemRectMin();
            const ImVec2 rowMax = ImGui::GetItemRectMax();
            ImGui::PopStyleColor(3);

            ImGui::GetWindowDrawList()->AddRect(
                rowMin, rowMax, ImGui::ColorConvertFloat4ToU32(WithAlpha(rarityColor, 0.85f)),
                5.0f, 0, 1.0f);

            if (rowPressed) {
                std::lock_guard<std::mutex> lock(gSelectionMutex);
                gSelectedSkin = *skin;
                gSelectedName = skin->itemName;
            }
            ImGui::SameLine(ImGui::GetWindowWidth() - 155.0f * scale);
            ImGui::TextColored(rarityColor, "%s", RarityName(rarity));
            ImGui::PopID();
        }
    }
    ImGui::EndChild();

    bool canApply = false;
    itemInfo pending;
    {
        std::lock_guard<std::mutex> lock(gSelectionMutex);
        canApply = !gSelectedName.empty();
        if (canApply) pending = gSelectedSkin;
    }
    if (!canApply) ImGui::BeginDisabled();
    if (ImGui::Button("APPLY SELECTED SKIN", ImVec2(-1, buttonH))) SelectSkin(pending);
    if (!canApply) ImGui::EndDisabled();

    DrawDetachedSearchKeyboard("##SKINHAX_KEYBOARD", "SEARCH WEAPON SKIN",
                               searchQuery, sizeof(searchQuery), &gSearchKeyboard);
}

} // namespace Eclipse