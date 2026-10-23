#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Fields.h"
#include "SkinHaxSystem.h"

namespace OblivionCharacterSkin {

// Character-Skin-only RVAs migrated from SRC2. ItemResource is deliberately not
// hooked here: SRC1's existing SkinHax collector already owns that constructor.
static constexpr uintptr_t kCharacterModelCtor = 0xB1A9738;
static constexpr uintptr_t kRoleConfCtor       = 0x90F20BC;
static constexpr uintptr_t kRolePackConfCtor   = 0x90F784C;
static constexpr uintptr_t kRoleConfNameGetter = 0x90F4214;

using Constructor = void (*)(void*);
using NameGetter = String* (*)(void*);

inline std::mutex gInstanceMutex;
inline std::mutex gDatabaseMutex;
inline std::mutex gSelectionMutex;
inline std::mutex gInstallMutex;

inline std::vector<void*> gCharacterModelInstances;
inline std::vector<void*> gRoleConfInstances;
inline std::vector<void*> gRolePackInstances;

inline Constructor gOrigCharacterModel = nullptr;
inline Constructor gOrigRoleConf = nullptr;
inline Constructor gOrigRolePack = nullptr;

inline std::atomic<bool> gInstalled{false};
inline std::atomic<bool> gWorkerStarted{false};
inline std::atomic<bool> gEarlyCaptureThreadStarted{false};
inline std::atomic<bool> gHookInstallOk{false};
inline std::atomic<int> gCharacterHookState{0}; // 0 pending, 1 installed, -1 invalid target, -2 Dobby failure
inline std::atomic<int> gRoleHookState{0};
inline std::atomic<int> gPackHookState{0};
inline std::atomic<bool> gScanRequested{false};
inline std::atomic<bool> gDatabaseReady{false};
inline size_t gLastBuiltFingerprint = 0;
inline size_t gLastUiFingerprint = 0;

// The current game build can move the late CharacterModel / ItemResource
// link fields without changing the constructors themselves.  Keep the old
// SRC2 offsets as defaults, but verify them against the live captured data and
// auto-discover the new pair when the old join produces no matches.
struct RuntimeJoinLayout {
    size_t characterItemAbs = 0x78;  // CharacterModel.ItemID (absolute object offset)
    size_t itemAvatarAbs = 0x40;     // ItemResource.AvatarModelID
    size_t itemIdAbs = 0x14;         // ItemResource.ID
    size_t roleIdAbs = 0x34;         // RoleConf.ID
    size_t avatarJoinScore = 0;
    size_t roleJoinScore = 0;
    size_t chainScore = 0;
    size_t nameScore = 0;
    size_t fingerprint = 0;
};

inline std::mutex gLayoutMutex;
inline RuntimeJoinLayout gJoinLayout{};

inline std::vector<charInfo> gCharacterDatabase;
inline std::vector<TargetChar> gTargetCharacters;
inline int gSelectedTargetIndex = 0;
inline bool gHasSelectedCharacter = false;
inline charInfo gSelectedCharacter{};
inline std::string gSelectedCharacterName;

inline char gSkinSearch[256] = "";
inline char gTargetSearch[256] = "";
inline bool gSkinKeyboard = false;
inline bool gTargetKeyboard = false;

static bool Valid(void* p) {
    return p && Tools::IsPtrValid(p);
}

static void Capture(std::vector<void*>& out, void* instance) {
    if (!Valid(instance)) return;
    std::lock_guard<std::mutex> lock(gInstanceMutex);
    if (std::find(out.begin(), out.end(), instance) == out.end())
        out.push_back(instance);
}

static void HookCharacterModel(void* p) {
    if (gOrigCharacterModel) gOrigCharacterModel(p);
    Capture(gCharacterModelInstances, p);
}

static void HookRoleConf(void* p) {
    if (gOrigRoleConf) gOrigRoleConf(p);
    Capture(gRoleConfInstances, p);
}

static void HookRolePack(void* p) {
    if (gOrigRolePack) gOrigRolePack(p);
    Capture(gRolePackInstances, p);
}

struct Snapshot {
    std::vector<void*> characterModels;
    std::vector<void*> roleConfs;
    std::vector<void*> rolePacks;
    std::vector<void*> itemResources;
};

static Snapshot GetSnapshot() {
    Snapshot out;
    {
        std::lock_guard<std::mutex> lock(gInstanceMutex);
        out.characterModels = gCharacterModelInstances;
        out.roleConfs = gRoleConfInstances;
        out.rolePacks = gRolePackInstances;
    }
    {
        // Reuse SRC1's single ItemResource hook instead of double-hooking the
        // same constructor and risking a duplicate/recursive trampoline.
        std::lock_guard<std::mutex> lock(OblivionSkinHax::gInstanceMutex);
        out.itemResources = OblivionSkinHax::gResourceInstances;
    }
    return out;
}

static size_t Fingerprint() {
    const Snapshot s = GetSnapshot();
    size_t value = s.characterModels.size();
    value = value * 1315423911u + s.roleConfs.size();
    value = value * 1315423911u + s.rolePacks.size();
    value = value * 1315423911u + s.itemResources.size();
    return value;
}

template <typename T>
static T ReadCapturedField(void* instance, size_t absoluteOffset) {
    T value{};
    if (!Valid(instance)) return value;
    // These are persistent config objects captured from their constructors.  A
    // memcpy avoids alignment UB while keeping the runtime offset discovery
    // independent of the compile-time struct layout.
    std::memcpy(&value,
                reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(instance) + absoluteOffset),
                sizeof(T));
    return value;
}

static bool PlausibleLink64(uint64_t value) {
    // Config/asset IDs in this path are numeric IDs, not process pointers.
    // The upper bound filters normal 64-bit Android addresses while allowing
    // future ID ranges far beyond the current values.
    return value != 0 && value < 1000000000000ULL;
}

static bool PlausibleId32(uint32_t value) {
    return value != 0 && value < 0x7FFFFFFFu;
}

static size_t ScoreAvatarJoin(const Snapshot& s, size_t characterOffset, size_t resourceOffset) {
    std::unordered_set<uint64_t> resourceValues;
    resourceValues.reserve(s.itemResources.size() / 2 + 64);
    for (void* res : s.itemResources) {
        const uint64_t value = ReadCapturedField<uint64_t>(res, resourceOffset);
        if (PlausibleLink64(value)) resourceValues.insert(value);
    }
    if (resourceValues.empty()) return 0;

    size_t score = 0;
    for (void* character : s.characterModels) {
        const uint64_t value = ReadCapturedField<uint64_t>(character, characterOffset);
        if (PlausibleLink64(value) && resourceValues.find(value) != resourceValues.end())
            ++score;
    }
    return score;
}

static size_t ScoreRoleJoin(const Snapshot& s, size_t resourceIdOffset, size_t roleIdOffset) {
    std::unordered_set<uint32_t> resourceIds;
    resourceIds.reserve(s.itemResources.size() / 2 + 64);
    for (void* res : s.itemResources) {
        const uint32_t value = ReadCapturedField<uint32_t>(res, resourceIdOffset);
        if (PlausibleId32(value)) resourceIds.insert(value);
    }
    if (resourceIds.empty()) return 0;

    size_t score = 0;
    for (void* role : s.roleConfs) {
        const uint32_t value = ReadCapturedField<uint32_t>(role, roleIdOffset);
        if (PlausibleId32(value) && resourceIds.find(value) != resourceIds.end())
            ++score;
    }
    return score;
}

// Score the relationship that actually matters:
// CharacterModel -> ItemResource -> RoleConf.
// The earlier detector maximized the first and second joins independently.
// On current builds that can select two unrelated high-cardinality fields and
// report thousands of pairwise matches while producing zero real characters.
static size_t ScoreFullChain(const Snapshot& s,
                             size_t characterOffset,
                             size_t resourceAvatarOffset,
                             size_t resourceIdOffset,
                             size_t roleIdOffset) {
    std::unordered_set<uint32_t> roleIds;
    roleIds.reserve(s.roleConfs.size() + 32);
    for (void* role : s.roleConfs) {
        const uint32_t id = ReadCapturedField<uint32_t>(role, roleIdOffset);
        if (PlausibleId32(id)) roleIds.insert(id);
    }
    if (roleIds.empty()) return 0;

    // Multiple ItemResource rows can share one AvatarModelID.  Keep an avatar
    // when AT LEAST ONE resource with that avatar has an ID that exists in the
    // RoleConf table.  Do not collapse the resource table to a single pointer.
    std::unordered_set<uint64_t> avatarsWithRole;
    avatarsWithRole.reserve(s.characterModels.size() * 2 + 64);
    for (void* res : s.itemResources) {
        const uint64_t avatar = ReadCapturedField<uint64_t>(res, resourceAvatarOffset);
        if (!PlausibleLink64(avatar)) continue;
        const uint32_t itemId = ReadCapturedField<uint32_t>(res, resourceIdOffset);
        if (!PlausibleId32(itemId)) continue;
        if (roleIds.find(itemId) != roleIds.end()) avatarsWithRole.insert(avatar);
    }
    if (avatarsWithRole.empty()) return 0;

    size_t score = 0;
    for (void* character : s.characterModels) {
        const uint64_t avatar = ReadCapturedField<uint64_t>(character, characterOffset);
        if (PlausibleLink64(avatar) && avatarsWithRole.find(avatar) != avatarsWithRole.end())
            ++score;
    }
    return score;
}

static RuntimeJoinLayout ResolveJoinLayout(const Snapshot& s, size_t fingerprint) {
    RuntimeJoinLayout cached;
    {
        std::lock_guard<std::mutex> lock(gLayoutMutex);
        cached = gJoinLayout;
        // A one-off accidental chain must never freeze the discovered layout.
        // Only reuse a layout when it has enough end-to-end coverage to be a
        // believable Character -> Resource -> Role relationship.
        const size_t minStableChain = std::max<size_t>(16, s.characterModels.size() / 20);
        if (cached.fingerprint == fingerprint && cached.chainScore >= minStableChain)
            return cached;
    }

    struct IdFieldSet {
        size_t offset = 0;
        std::unordered_set<uint32_t> values;
        size_t validCount = 0;
    };
    struct CharacterField {
        size_t offset = 0;
        std::vector<uint64_t> values;
        size_t validCount = 0;
        size_t uniqueCount = 0;
    };
    struct RoleCandidate {
        size_t resourceIdOffset = 0;
        size_t roleIdOffset = 0;
        size_t score = 0;
        size_t roleUnique = 0;
        size_t resourceUnique = 0;
    };

    // Precompute plausible 32-bit ID fields.  RoleConf.ID is expected to be
    // highly unique across the RoleConf table; this filters enum/bool fields
    // which previously produced misleadingly large pairwise scores.
    std::vector<IdFieldSet> roleFields;
    for (size_t roleOffset = 0x20; roleOffset <= 0x90; roleOffset += 4) {
        IdFieldSet f;
        f.offset = roleOffset;
        f.values.reserve(s.roleConfs.size() + 16);
        for (void* role : s.roleConfs) {
            const uint32_t value = ReadCapturedField<uint32_t>(role, roleOffset);
            if (!PlausibleId32(value)) continue;
            ++f.validCount;
            f.values.insert(value);
        }
        const size_t minUnique = std::max<size_t>(16, s.roleConfs.size() / 3);
        if (f.validCount >= minUnique && f.values.size() >= minUnique)
            roleFields.push_back(std::move(f));
    }

    std::vector<IdFieldSet> resourceIdFields;
    for (size_t resourceOffset = 0x10; resourceOffset <= 0x60; resourceOffset += 4) {
        IdFieldSet f;
        f.offset = resourceOffset;
        f.values.reserve(s.itemResources.size() / 2 + 64);
        for (void* res : s.itemResources) {
            const uint32_t value = ReadCapturedField<uint32_t>(res, resourceOffset);
            if (!PlausibleId32(value)) continue;
            ++f.validCount;
            f.values.insert(value);
        }
        // ItemResource.ID has a large domain.  Ignore tiny enum-like fields.
        if (f.values.size() >= 64)
            resourceIdFields.push_back(std::move(f));
    }

    std::vector<RoleCandidate> roleCandidates;
    roleCandidates.reserve(roleFields.size() * resourceIdFields.size());
    for (const auto& resField : resourceIdFields) {
        for (const auto& roleField : roleFields) {
            size_t score = 0;
            for (uint32_t id : roleField.values) {
                if (resField.values.find(id) != resField.values.end()) ++score;
            }
            if (score < 8) continue;
            roleCandidates.push_back({resField.offset, roleField.offset, score,
                                      roleField.values.size(), resField.values.size()});
        }
    }

    std::sort(roleCandidates.begin(), roleCandidates.end(),
              [](const RoleCandidate& a, const RoleCandidate& b) {
                  if (a.score != b.score) return a.score > b.score;
                  if (a.roleUnique != b.roleUnique) return a.roleUnique > b.roleUnique;
                  return a.resourceUnique > b.resourceUnique;
              });

    // Do not discard the real ID pair merely because several unrelated fields
    // happen to have a slightly larger global intersection.  The old top-12
    // pruning was the reason the detector could settle on a one-skin chain.
    constexpr size_t kMaxRoleCandidates = 64;
    if (roleCandidates.size() > kMaxRoleCandidates)
        roleCandidates.resize(kMaxRoleCandidates);

    // Always include the historical/current cached ID pair as a candidate.
    const size_t cachedRoleScore = ScoreRoleJoin(s, cached.itemIdAbs, cached.roleIdAbs);
    if (cachedRoleScore > 0) {
        const bool already = std::any_of(roleCandidates.begin(), roleCandidates.end(),
            [&](const RoleCandidate& c) {
                return c.resourceIdOffset == cached.itemIdAbs && c.roleIdOffset == cached.roleIdAbs;
            });
        if (!already)
            roleCandidates.push_back({cached.itemIdAbs, cached.roleIdAbs, cachedRoleScore, 0, 0});
    }

    // Precompute every plausible CharacterModel 64-bit link field.  The real
    // ItemID/Avatar link is expected to exist for a substantial portion of the
    // CharacterModel table and to have many distinct values.
    std::vector<CharacterField> characterFields;
    for (size_t characterOffset = 0x48; characterOffset <= 0xC0; characterOffset += 8) {
        CharacterField f;
        f.offset = characterOffset;
        f.values.reserve(s.characterModels.size());
        std::unordered_set<uint64_t> unique;
        unique.reserve(s.characterModels.size() + 16);
        for (void* character : s.characterModels) {
            const uint64_t value = ReadCapturedField<uint64_t>(character, characterOffset);
            f.values.push_back(value);
            if (!PlausibleLink64(value)) continue;
            ++f.validCount;
            unique.insert(value);
        }
        f.uniqueCount = unique.size();
        const size_t minUseful = std::max<size_t>(16, s.characterModels.size() / 10);
        if (f.validCount >= minUseful && f.uniqueCount >= minUseful)
            characterFields.push_back(std::move(f));
    }

    RuntimeJoinLayout best = cached;
    best.avatarJoinScore = 0;
    best.roleJoinScore = 0;
    best.chainScore = 0;
    best.nameScore = 0;
    best.fingerprint = fingerprint;

    size_t bestTieScore = 0;
    const size_t minAcceptChain = std::max<size_t>(16, s.characterModels.size() / 20);

    // Critical difference from the previous detector:
    //   1) choose a plausible Resource.ID <-> Role.ID relation;
    //   2) keep ONLY resource rows whose ID really exists in that Role table;
    //   3) search every Avatar field and Character link field on that subset.
    // This scores the complete relationship instead of combining two unrelated
    // high-scoring pairwise joins.
    for (const RoleCandidate& rc : roleCandidates) {
        std::unordered_set<uint32_t> roleIds;
        roleIds.reserve(s.roleConfs.size() + 16);
        for (void* role : s.roleConfs) {
            const uint32_t id = ReadCapturedField<uint32_t>(role, rc.roleIdOffset);
            if (PlausibleId32(id)) roleIds.insert(id);
        }
        if (roleIds.empty()) continue;

        std::vector<void*> roleBackedResources;
        roleBackedResources.reserve(roleIds.size() * 2 + 16);
        for (void* res : s.itemResources) {
            const uint32_t id = ReadCapturedField<uint32_t>(res, rc.resourceIdOffset);
            if (PlausibleId32(id) && roleIds.find(id) != roleIds.end())
                roleBackedResources.push_back(res);
        }
        if (roleBackedResources.size() < minAcceptChain) continue;

        for (size_t resourceAvatarOffset = 0x30; resourceAvatarOffset <= 0xA0; resourceAvatarOffset += 8) {
            std::unordered_set<uint64_t> avatars;
            avatars.reserve(roleBackedResources.size() * 2 + 16);
            for (void* res : roleBackedResources) {
                const uint64_t avatar = ReadCapturedField<uint64_t>(res, resourceAvatarOffset);
                if (PlausibleLink64(avatar)) avatars.insert(avatar);
            }
            if (avatars.size() < minAcceptChain) continue;

            for (const CharacterField& cf : characterFields) {
                size_t chain = 0;
                for (uint64_t avatar : cf.values) {
                    if (PlausibleLink64(avatar) && avatars.find(avatar) != avatars.end())
                        ++chain;
                }

                if (chain < minAcceptChain) continue;

                // Prefer the chain that covers the most CharacterModel rows.
                // Pairwise ID coverage is only a tie-breaker now.
                const size_t tie = rc.score + cf.uniqueCount + avatars.size();
                if (chain > best.chainScore ||
                    (chain == best.chainScore && tie > bestTieScore)) {
                    best.characterItemAbs = cf.offset;
                    best.itemAvatarAbs = resourceAvatarOffset;
                    best.itemIdAbs = rc.resourceIdOffset;
                    best.roleIdAbs = rc.roleIdOffset;
                    best.roleJoinScore = rc.score;
                    best.chainScore = chain;
                    bestTieScore = tie;
                }
            }
        }
    }

    if (best.chainScore >= minAcceptChain) {
        best.avatarJoinScore = ScoreAvatarJoin(s, best.characterItemAbs, best.itemAvatarAbs);
    } else {
        // Never publish a 1-row/2-row accidental database.  Keep the strongest
        // diagnostic pair but force BuildDatabase to continue retrying.
        best.chainScore = 0;
        if (!roleCandidates.empty()) {
            best.itemIdAbs = roleCandidates.front().resourceIdOffset;
            best.roleIdAbs = roleCandidates.front().roleIdOffset;
            best.roleJoinScore = roleCandidates.front().score;
        }
        best.avatarJoinScore = ScoreAvatarJoin(s, best.characterItemAbs, best.itemAvatarAbs);
    }

    {
        std::lock_guard<std::mutex> lock(gLayoutMutex);
        gJoinLayout = best;
    }
    return best;
}

static RuntimeJoinLayout JoinLayoutSnapshot() {
    std::lock_guard<std::mutex> lock(gLayoutMutex);
    return gJoinLayout;
}

static std::string GetRoleName(void* roleConf) {
    if (!Valid(roleConf)) return {};
    const uintptr_t base = Tools::GetBaseAddress("libunity.so");
    if (!base) return {};

    auto getter = reinterpret_cast<NameGetter>(base + kRoleConfNameGetter);
    if (!Valid(reinterpret_cast<void*>(getter))) return {};
    String* value = getter(roleConf);
    if (!Valid(value)) return {};
    const char* text = value->CString();
    return text ? std::string(text) : std::string();
}

static const char* RarityPrefix(int colorId) {
    switch (colorId) {
        case 5: return "[M] ";
        case 4: return "[L] ";
        case 3: return "[E] ";
        default: return "[C] ";
    }
}

static bool BuildDatabase() {
    const Snapshot s = GetSnapshot();
    if (s.characterModels.empty() || s.itemResources.empty() || s.roleConfs.empty()) {
        gDatabaseReady.store(false, std::memory_order_release);
        return false;
    }

    // Do not run the wider runtime layout probe on tiny/incomplete startup
    // snapshots.  The real config tables populate very quickly and the worker
    // will retry automatically.
    if (s.characterModels.size() < 8 || s.itemResources.size() < 64 || s.roleConfs.size() < 8) {
        gDatabaseReady.store(false, std::memory_order_release);
        return false;
    }

    size_t fingerprint = s.characterModels.size();
    fingerprint = fingerprint * 1315423911u + s.roleConfs.size();
    fingerprint = fingerprint * 1315423911u + s.rolePacks.size();
    fingerprint = fingerprint * 1315423911u + s.itemResources.size();
    RuntimeJoinLayout layout = ResolveJoinLayout(s, fingerprint);

    if (layout.avatarJoinScore == 0 || layout.roleJoinScore == 0 || layout.chainScore == 0) {
        gDatabaseReady.store(false, std::memory_order_release);
        return false;
    }

    std::unordered_map<uint64_t, std::vector<void*>> itemResByAvatarModelId;
    std::unordered_map<int, void*> roleConfById;
    std::unordered_map<int, void*> rolePackByCharId;

    itemResByAvatarModelId.reserve(s.itemResources.size() / 2 + 64);
    roleConfById.reserve(s.roleConfs.size() + 16);

    for (void* res : s.itemResources) {
        if (!Valid(res)) continue;
        const uint64_t avatarId = ReadCapturedField<uint64_t>(res, layout.itemAvatarAbs);
        if (PlausibleLink64(avatarId)) itemResByAvatarModelId[avatarId].push_back(res);
    }

    for (void* conf : s.roleConfs) {
        if (!Valid(conf)) continue;
        const uint32_t id = ReadCapturedField<uint32_t>(conf, layout.roleIdAbs);
        if (PlausibleId32(id)) roleConfById[static_cast<int>(id)] = conf;
    }

    // RolePack is optional for constructing the character list.  If its layout
    // has not moved, preserve all SRC2 background/entry data; otherwise skins
    // can still load and apply their core CharacterModel/ItemResource/RoleConf
    // data instead of the entire database being rejected.
    for (void* pack : s.rolePacks) {
        if (!Valid(pack)) continue;
        auto* fields = reinterpret_cast<RolePackFields*>(reinterpret_cast<uintptr_t>(pack) + 0x10);
        if (!Valid(fields)) continue;
        if (fields->mMatchRoomEasterEggCharacterID != 0)
            rolePackByCharId[static_cast<int>(fields->mMatchRoomEasterEggCharacterID)] = pack;
    }

    std::vector<charInfo> rebuiltCharacters;
    std::vector<TargetChar> rebuiltTargets;
    rebuiltCharacters.reserve(s.characterModels.size());
    rebuiltTargets.reserve(s.characterModels.size());

    size_t chainMatches = 0;
    size_t namedMatches = 0;
    std::unordered_set<uint64_t> seenCharacterKeys;
    seenCharacterKeys.reserve(s.characterModels.size() + 16);

    for (void* charModel : s.characterModels) {
        if (!Valid(charModel)) continue;
        auto* charFields = reinterpret_cast<CharacterModelFields*>(reinterpret_cast<uintptr_t>(charModel) + 0x10);
        if (!Valid(charFields)) continue;

        const uint64_t avatarModelId = ReadCapturedField<uint64_t>(charModel, layout.characterItemAbs);
        if (!PlausibleLink64(avatarModelId)) continue;

        auto itemIt = itemResByAvatarModelId.find(avatarModelId);
        if (itemIt == itemResByAvatarModelId.end()) continue;

        // Current resource tables can contain several rows with the same
        // AvatarModelID. The old unordered_map<avatar, pointer> silently kept
        // only the last row, which is exactly how we got 1903 + 2128 pairwise
        // matches but a zero end-to-end chain. Pick a resource whose runtime ID
        // really exists in RoleConf instead.
        void* itemResource = nullptr;
        void* roleConf = nullptr;
        uint32_t itemIdRaw = 0;
        for (void* candidate : itemIt->second) {
            if (!Valid(candidate)) continue;
            const uint32_t candidateId = ReadCapturedField<uint32_t>(candidate, layout.itemIdAbs);
            if (!PlausibleId32(candidateId)) continue;
            auto roleIt = roleConfById.find(static_cast<int>(candidateId));
            if (roleIt == roleConfById.end()) continue;
            itemResource = candidate;
            roleConf = roleIt->second;
            itemIdRaw = candidateId;
            break;
        }
        if (!itemResource || !roleConf) continue;

        auto* itemFields = reinterpret_cast<ItemResourceFields*>(reinterpret_cast<uintptr_t>(itemResource) + 0x10);
        auto* roleFields = reinterpret_cast<RoleConfFields*>(reinterpret_cast<uintptr_t>(roleConf) + 0x10);
        if (!Valid(itemFields) || !Valid(roleFields)) continue;
        const int itemId = static_cast<int>(itemIdRaw);

        ++chainMatches;

        // Avoid duplicate constructor/config entries for the exact same source
        // character while preserving distinct skins/items.
        const uint64_t dedupeKey = (static_cast<uint64_t>(itemIdRaw) << 32) ^ avatarModelId;
        if (!seenCharacterKeys.insert(dedupeKey).second) continue;

        int entryAnimId = 0;
        int gestureId = 0;
        int handEffectUi = 0;
        int loadingFrame = 0;
        int killStreakSkinId = 0;
        int dynamicRoomAssetId = 0;
        int lobbySceneType = 0;
        int entryUiEffectId = 0;

        auto packIt = rolePackByCharId.find(itemId);
        if (packIt != rolePackByCharId.end()) {
            auto* packFields = reinterpret_cast<RolePackFields*>(reinterpret_cast<uintptr_t>(packIt->second) + 0x10);
            if (Valid(packFields)) {
                entryAnimId = packFields->EntryAnimID;
                gestureId = packFields->GestureId;
                handEffectUi = packFields->HandEffectUI;
                loadingFrame = packFields->LoadingFrame;
                killStreakSkinId = packFields->KillStreakSkinID;
                dynamicRoomAssetId = packFields->DynamicRoomAssetID;
                lobbySceneType = packFields->LobbySceneType;
                entryUiEffectId = packFields->EntryUIEffectAssetID;
            }
        }

        std::string charName = GetRoleName(roleConf);
        if (!charName.empty()) {
            ++namedMatches;
        } else {
            // A moved/incorrect name getter should no longer make the complete
            // Character database look empty.  Keep the skin functional and make
            // the missing getter obvious in the debug diagnostics below.
            charName = std::string("Character ") + std::to_string(itemId);
        }

        charInfo info{};
        info.charName = std::string(RarityPrefix(roleFields->ColorID)) + charName;
        info.charModel = {
            charFields->BRBagModel, charFields->BRHeadModel, charFields->BRLobby,
            charFields->BRModel, charFields->BindEffect1P, charFields->ChangeClipEffect1P,
            charFields->DefaultModelID, charFields->Guarder1P, charFields->Guarder3P,
            charFields->GuarderBagModel, charFields->GuarderHeadModel, charFields->GuarderLobby
        };
        info.charRes = {
            itemFields->FxAssetID, itemFields->InventoryModelID, itemFields->ModelAssetIDRaw
        };
        info.charRes2 = {
            itemFields->UISmallSpriteName, itemFields->UIMiniSpriteName,
            itemFields->UISpriteName, itemFields->UISquareSpriteName
        };
        info.charRole = {
            static_cast<int>(roleFields->roleLeftArmID),
            static_cast<int>(roleFields->roleFinalSuitID),
            static_cast<int>(roleFields->roleBasicHologramID),
            static_cast<int>(roleFields->ColorID),
            static_cast<int>(roleFields->ColorSubID),
            static_cast<int>(roleFields->ShowRare),
            static_cast<int>(roleFields->RoleLvGroupID),
            roleFields->RolePackID
        };
        info.charRole2 = { roleFields->LOCID_Name }; {
 //       info.charPack = { 
            roleFields->RolePackID, entryAnimId, gestureId, handEffectUi,
            loadingFrame, killStreakSkinId, dynamicRoomAssetId,
            lobbySceneType, entryUiEffectId;
        };

        rebuiltCharacters.push_back(info);
        rebuiltTargets.push_back({
            charName,
            charFields->Traitor1P,
            charFields->Traitor3P,
            itemId,
            static_cast<int>(ReadCapturedField<uint32_t>(roleConf, layout.roleIdAbs)),
            roleFields->RolePackID
        });
    }

    {
        std::lock_guard<std::mutex> lock(gLayoutMutex);
        gJoinLayout.chainScore = chainMatches;
        gJoinLayout.nameScore = namedMatches;
    }

    // With thousands of captured Character/Role configs, a database containing
    // only one or two rows is an accidental field collision, not a successful
    // Character Skin load.  Reject suspiciously tiny databases so the worker
    // keeps searching instead of publishing a fake one-skin result.
    const size_t minDatabaseRows = std::max<size_t>(16, s.characterModels.size() / 20);
    if (rebuiltCharacters.size() < minDatabaseRows || rebuiltTargets.size() < minDatabaseRows) {
        gDatabaseReady.store(false, std::memory_order_release);
        return false;
    }

    int selectedIndex = 0;
    for (size_t i = 0; i < rebuiltTargets.size(); ++i) {
        if (ToLower(rebuiltTargets[i].name) == "charly" || rebuiltTargets[i].itemID == 100301208) {
            selectedIndex = static_cast<int>(i);
            break;
        }
    }

    {
        std::lock_guard<std::mutex> lock(gDatabaseMutex);
        gCharacterDatabase = rebuiltCharacters;
        gTargetCharacters = rebuiltTargets;
        if (gSelectedTargetIndex < 0 || gSelectedTargetIndex >= static_cast<int>(gTargetCharacters.size()))
            gSelectedTargetIndex = selectedIndex;
        else if (!gDatabaseReady.load(std::memory_order_acquire))
            gSelectedTargetIndex = selectedIndex;

        // Keep the pre-existing Fields.h containers synchronized so any other
        // SRC1 code that already references them sees the same migrated dataset.
        charData = gCharacterDatabase;
        g_targetCharacters = gTargetCharacters;
        g_selectedTargetCharIndex = gSelectedTargetIndex;
        loadCharacter = true;
    }

    gDatabaseReady.store(true, std::memory_order_release);
    return true;
}

static void CopyCharacterModel(CharacterModelFields* target, const charInfo& source) {
    if (!Valid(target)) return;
    target->BRBagModel = source.charModel[0];
    target->BRHeadModel = source.charModel[1];
    target->BRLobby = source.charModel[2];
    target->BRModel = source.charModel[3];
    target->BindEffect1P = source.charModel[4];
    target->ChangeClipEffect1P = source.charModel[5];
    target->DefaultModelID = source.charModel[6];
    target->Guarder1P = source.charModel[7];
    target->Guarder3P = source.charModel[8];
    target->GuarderBagModel = source.charModel[9];
    target->GuarderHeadModel = source.charModel[10];
    target->GuarderLobby = source.charModel[11];
}

static bool ApplyCharacterSkin(const charInfo& source) {
    const Snapshot s = GetSnapshot();
    TargetChar target{};
    {
        std::lock_guard<std::mutex> lock(gDatabaseMutex);
        if (gTargetCharacters.empty() || gSelectedTargetIndex < 0 ||
            gSelectedTargetIndex >= static_cast<int>(gTargetCharacters.size()))
            return false;
        target = gTargetCharacters[gSelectedTargetIndex];
        g_selectedTargetCharIndex = gSelectedTargetIndex;
    }

    const RuntimeJoinLayout layout = JoinLayoutSnapshot();

    const bool isCharly = ToLower(target.name) == "charly" || target.itemID == 100301208;
    const int selectedTraitor1P = isCharly ? 710001101 : target.traitor1p;
    const int selectedTraitor3P = isCharly ? 710001102 : target.traitor3p;
    const int selectedItemId = isCharly ? 100301208 : target.itemID;
    const int selectedRoleId = isCharly ? 100301208 : target.roleID;

    CharacterModelFields* targetCharacter1P = nullptr;
    CharacterModelFields* fallback1P = nullptr;
    CharacterModelFields* targetCharacter3P = nullptr;
    CharacterModelFields* fallback3P = nullptr;

    for (void* charModel : s.characterModels) {
        if (!Valid(charModel)) continue;
        auto* fields = reinterpret_cast<CharacterModelFields*>(reinterpret_cast<uintptr_t>(charModel) + 0x10);
        if (!Valid(fields)) continue;
        if (fields->Traitor1P == 710001101) fallback1P = fields;
        if (fields->Traitor3P == 710001102) fallback3P = fields;
        if (fields->Traitor1P == selectedTraitor1P) targetCharacter1P = fields;
        if (fields->Traitor3P == selectedTraitor3P) targetCharacter3P = fields;
    }
    if (!targetCharacter1P) targetCharacter1P = fallback1P;
    if (!targetCharacter3P) targetCharacter3P = fallback3P;
    CopyCharacterModel(targetCharacter1P, source);
    CopyCharacterModel(targetCharacter3P, source);

    ItemResourceFields* targetItem = nullptr;
    ItemResourceFields* fallbackItem = nullptr;
    for (void* itemRes : s.itemResources) {
        if (!Valid(itemRes)) continue;
        auto* fields = reinterpret_cast<ItemResourceFields*>(reinterpret_cast<uintptr_t>(itemRes) + 0x10);
        if (!Valid(fields)) continue;
        const int runtimeItemId = static_cast<int>(ReadCapturedField<uint32_t>(itemRes, layout.itemIdAbs));
        if (runtimeItemId == 100301208) fallbackItem = fields;
        if (runtimeItemId == selectedItemId) {
            targetItem = fields;
            break;
        }
    }
    if (!targetItem) targetItem = fallbackItem;
    if (targetItem) {
        targetItem->FxAssetID = source.charRes[0];
        targetItem->InventoryModelID = source.charRes[1];
        targetItem->ModelAssetIDRaw = source.charRes[2];
        targetItem->UISmallSpriteName = source.charRes2[0];
        targetItem->UIMiniSpriteName = source.charRes2[1];
        targetItem->UISpriteName = source.charRes2[2];
        targetItem->UISquareSpriteName = source.charRes2[3];
    }

    RoleConfFields* targetRole = nullptr;
    RoleConfFields* fallbackRole = nullptr;
    for (void* role : s.roleConfs) {
        if (!Valid(role)) continue;
        auto* fields = reinterpret_cast<RoleConfFields*>(reinterpret_cast<uintptr_t>(role) + 0x10);
        if (!Valid(fields)) continue;
        const int runtimeRoleId = static_cast<int>(ReadCapturedField<uint32_t>(role, layout.roleIdAbs));
        if (runtimeRoleId == 100301208) fallbackRole = fields;
        if (runtimeRoleId == selectedRoleId) {
            targetRole = fields;
            break;
        }
    }
    if (!targetRole) targetRole = fallbackRole;
    if (targetRole) {
        targetRole->roleLeftArmID = static_cast<uint>(source.charRole[0]);
        targetRole->roleFinalSuitID = static_cast<uint>(source.charRole[1]);
        targetRole->roleBasicHologramID = static_cast<uint>(source.charRole[2]);
        targetRole->ColorID = static_cast<uint8_t>(source.charRole[3]);
        targetRole->ColorSubID = static_cast<uint8_t>(source.charRole[4]);
        targetRole->ShowRare = source.charRole[5] != 0;
        targetRole->RoleLvGroupID = static_cast<uint8_t>(source.charRole[6]);
        targetRole->RolePackID = source.charRole[7];
        targetRole->LOCID_Name = source.charRole2[0];
    }

    for (void* pack : s.rolePacks) {
        if (!Valid(pack)) continue;
        auto* fields = reinterpret_cast<RolePackFields*>(reinterpret_cast<uintptr_t>(pack) + 0x10);
        if (!Valid(fields) || static_cast<int>(fields->RolePackID) != target.rolepackID) continue;
        fields->EntryAnimID = source.charPack[1];
        fields->GestureId = source.charPack[2];
        fields->HandEffectUI = source.charPack[3];
        fields->LoadingFrame = source.charPack[4];
        fields->KillStreakSkinID = source.charPack[5];
        if (source.charPack[6] > 0) fields->DynamicRoomAssetID = source.charPack[6];
        if (source.charPack[7] > 0) fields->LobbySceneType = source.charPack[7];
        if (source.charPack[8] > 0) fields->EntryUIEffectAssetID = source.charPack[8];
    }

    return targetCharacter1P || targetCharacter3P || targetItem || targetRole;
}

static bool InstallOne(uintptr_t base, uintptr_t rva, void* hook, void** orig,
                       std::atomic<int>& state) {
    if (orig && *orig) {
        state.store(1, std::memory_order_release);
        return true;
    }
    void* target = reinterpret_cast<void*>(base + rva);
    if (!Valid(target)) {
        state.store(-1, std::memory_order_release);
        return false;
    }
    const int result = DobbyHook(target, hook, orig);
    state.store(result == 0 ? 1 : -2, std::memory_order_release);
    return result == 0;
}

static void Worker() {
    size_t previous = 0;
    int stablePasses = 0;
    while (gInstalled.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (!gScanRequested.load(std::memory_order_acquire)) continue;

        const size_t current = Fingerprint();
        if (current == 0) {
            previous = 0;
            stablePasses = 0;
            continue;
        }
        if (current == previous) ++stablePasses;
        else stablePasses = 0;
        previous = current;
        if (stablePasses < 1 || current == gLastBuiltFingerprint) continue;

        if (BuildDatabase()) {
            gLastBuiltFingerprint = current;
            charInfo selected{};
            bool apply = false;
            {
                std::lock_guard<std::mutex> lock(gSelectionMutex);
                if (gHasSelectedCharacter) {
                    selected = gSelectedCharacter;
                    apply = true;
                }
            }
            if (apply) ApplyCharacterSkin(selected);
        }
    }
}

inline bool InstallCaptureOnly() {
    if (gHookInstallOk.load(std::memory_order_acquire)) return true;

    // Installation can be requested by both the early JNI thread and the normal
    // hook thread. Serialize it so the same constructor can never be patched
    // twice at the same time.
    std::lock_guard<std::mutex> installLock(gInstallMutex);
    if (gHookInstallOk.load(std::memory_order_acquire)) return true;

    const uintptr_t base = Tools::GetBaseAddress("libunity.so");
    if (!base) {
        gInstalled.store(false, std::memory_order_release);
        return false;
    }

    // Match SRC2's working order: arm ItemResource first, then install the three
    // Character-Skin constructor captures immediately from the same live Unity
    // mapping. There is no metadata scan or sleep loop on the render thread.
    OblivionSkinHax::InstallCaptureOnly();

    const bool characterHook = InstallOne(base, kCharacterModelCtor,
                                          reinterpret_cast<void*>(HookCharacterModel),
                                          (void**)&gOrigCharacterModel, gCharacterHookState);
    const bool roleHook = InstallOne(base, kRoleConfCtor,
                                     reinterpret_cast<void*>(HookRoleConf),
                                     (void**)&gOrigRoleConf, gRoleHookState);
    const bool packHook = InstallOne(base, kRolePackConfCtor,
                                     reinterpret_cast<void*>(HookRolePack),
                                     (void**)&gOrigRolePack, gPackHookState);

    const bool allHooksReady = characterHook && roleHook && packHook;
    gHookInstallOk.store(allHooksReady, std::memory_order_release);
    gInstalled.store(allHooksReady, std::memory_order_release);
    if (!allHooksReady) return false;

    // Build continuously in the background as soon as constructors start
    // arriving. Do not wait for the Character tab to be opened first.
    gScanRequested.store(true, std::memory_order_release);
    if (!gWorkerStarted.exchange(true, std::memory_order_acq_rel))
        std::thread(Worker).detach();
    return true;
}

inline void StartEarlyCapture() {
    // Mirror SRC2's actual Skins_Thread timing instead of hooking immediately
    // when libunity first appears. SRC2 lets Unity finish mapping/relocation,
    // waits for libanogs, then installs the config constructor hooks. Installing
    // these trampolines too early can be overwritten later during game startup.
    if (gEarlyCaptureThreadStarted.exchange(true, std::memory_order_acq_rel)) return;

    std::thread([] {
        while (!Tools::GetBaseAddress("libunity.so"))
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Exact settle window used by SRC2 before its skin-hook block.
        std::this_thread::sleep_for(std::chrono::seconds(3));

        // SRC2 waits for libanogs before arming the config constructors. Keep this
        // off the render thread so there is no menu freeze while the dependency
        // is loading.
        int anogsRetry = 0;
        while (!Tools::GetBaseAddress("libanogs.so") && anogsRetry < 60) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ++anogsRetry;
        }

        // Give the loader a small final settle window after libanogs becomes
        // visible, then perform one serialized install pass.
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        InstallCaptureOnly();
    }).detach();
}

inline std::vector<charInfo> DatabaseSnapshot() {
    std::lock_guard<std::mutex> lock(gDatabaseMutex);
    return gCharacterDatabase;
}

inline std::vector<TargetChar> TargetSnapshot() {
    std::lock_guard<std::mutex> lock(gDatabaseMutex);
    return gTargetCharacters;
}

inline void DrawPanel(float scale) {
    // Keep the full SRC2 Character UI visible even while capture is arming. The
    // old early-return is why Search Custom Char disappeared in the last build.
    // Initialization remains background-only; DrawPanel never sleeps or hooks.
    StartEarlyCapture();
    const bool hooksReady = gHookInstallOk.load(std::memory_order_acquire);
    if (!hooksReady) {
        if (!Tools::GetBaseAddress("libunity.so"))
            ImGui::TextDisabled("Character Skin: waiting for libunity.so...");
        else
            ImGui::TextDisabled("Character Skin: arming SRC2 capture hooks...");
    }

    gScanRequested.store(true, std::memory_order_release);
    // Database discovery runs only on Worker(). Do not perform the multi-table
    // layout scan from the ImGui/render thread.

    const float totalAvail = ImGui::GetContentRegionAvail().x;
    const float halfWidth = std::max(120.0f, (totalAvail - 10.0f * scale) * 0.5f);
    ImGui::TextUnformatted("Search Skin:");
    ImGui::SameLine(halfWidth + 10.0f * scale);
    ImGui::TextUnformatted("Search Custom Char:");

    ImGui::SetNextItemWidth(halfWidth);
    ImGui::InputText("##CHARACTER_SKIN_SEARCH", gSkinSearch, sizeof(gSkinSearch));
    if (ImGui::IsItemClicked() || ImGui::IsItemActive()) gSkinKeyboard = true;
    ImGui::SameLine(0.0f, 10.0f * scale);
    ImGui::SetNextItemWidth(halfWidth);
    ImGui::InputText("##CHARACTER_TARGET_SEARCH", gTargetSearch, sizeof(gTargetSearch));
    if (ImGui::IsItemClicked() || ImGui::IsItemActive()) gTargetKeyboard = true;

    const std::vector<TargetChar> targets = TargetSnapshot();
    if (!targets.empty()) {
        const std::string targetQuery = ToLower(std::string(gTargetSearch));
        std::vector<int> filteredIndices;
        filteredIndices.reserve(targets.size());
        for (size_t i = 0; i < targets.size(); ++i) {
            if (std::strlen(gTargetSearch) >= 3 &&
                ToLower(targets[i].name).find(targetQuery) == std::string::npos)
                continue;
            filteredIndices.push_back(static_cast<int>(i));
        }

        if (!filteredIndices.empty()) {
            int selectedTarget = 0;
            {
                std::lock_guard<std::mutex> lock(gDatabaseMutex);
                selectedTarget = gSelectedTargetIndex;
            }
            int filteredSelection = 0;
            for (size_t i = 0; i < filteredIndices.size(); ++i) {
                if (filteredIndices[i] == selectedTarget) {
                    filteredSelection = static_cast<int>(i);
                    break;
                }
            }

            const char* preview = targets[filteredIndices[filteredSelection]].name.c_str();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::BeginCombo("Custom character:", preview)) {
                for (size_t i = 0; i < filteredIndices.size(); ++i) {
                    const int realIndex = filteredIndices[i];
                    const bool selected = realIndex == selectedTarget;
                    if (ImGui::Selectable(targets[realIndex].name.c_str(), selected)) {
                        std::lock_guard<std::mutex> lock(gDatabaseMutex);
                        gSelectedTargetIndex = realIndex;
                        g_selectedTargetCharIndex = realIndex;
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        } else {
            ImGui::TextUnformatted("No matching characters");
        }
    } else {
        ImGui::TextUnformatted("Loading target characters...");
    }

    const std::vector<charInfo> db = DatabaseSnapshot();
    const std::string skinQuery = ToLower(std::string(gSkinSearch));
    ImGui::Separator();
    if (ImGui::BeginChild("##CharacterSkinList", ImVec2(0, -4.0f), true,
                          ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
        if (!db.empty()) {
            // Normally this table contains hundreds/thousands of entries.  Keep
            // a compact diagnostic visible only for a suspiciously small list.
            if (db.size() < 50) {
                const RuntimeJoinLayout layout = JoinLayoutSnapshot();
                ImGui::TextDisabled("Loaded %zu | C@0x%lX -> A@0x%lX | ResID@0x%lX -> RoleID@0x%lX | FullChain %zu | Names %zu",
                                    db.size(),
                                    (unsigned long)layout.characterItemAbs,
                                    (unsigned long)layout.itemAvatarAbs,
                                    (unsigned long)layout.itemIdAbs,
                                    (unsigned long)layout.roleIdAbs,
                                    layout.chainScore, layout.nameScore);
            }
            for (const auto& skin : db) {
                if (std::strlen(gSkinSearch) >= 3 &&
                    ToLower(skin.charName).find(skinQuery) == std::string::npos)
                    continue;

                bool checked = false;
                {
                    std::lock_guard<std::mutex> lock(gSelectionMutex);
                    checked = gHasSelectedCharacter && gSelectedCharacterName == skin.charName;
                }
                const bool wasChecked = checked;
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f * scale, 8.0f * scale));
                if (ImGui::Checkbox(skin.charName.c_str(), &checked)) {
                    {
                        std::lock_guard<std::mutex> lock(gSelectionMutex);
                        if (checked) {
                            gSelectedCharacter = skin;
                            gSelectedCharacterName = skin.charName;
                            gHasSelectedCharacter = true;
                        } else if (wasChecked) {
                            gSelectedCharacterName.clear();
                            gHasSelectedCharacter = false;
                        }
                    }
                    // SRC2 applies the clicked source on both checkbox transitions;
                    // unchecking does not restore a previous/default character.
                    ApplyCharacterSkin(skin);
                }
                ImGui::PopStyleVar();
            }
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No character skins loaded");
            const Snapshot debugSnapshot = GetSnapshot();
            ImGui::TextDisabled("Captured: Character %zu | Role %zu | Pack %zu | Resource %zu",
                                debugSnapshot.characterModels.size(), debugSnapshot.roleConfs.size(),
                                debugSnapshot.rolePacks.size(), debugSnapshot.itemResources.size());
            const int ch = gCharacterHookState.load(std::memory_order_acquire);
            const int rh = gRoleHookState.load(std::memory_order_acquire);
            const int ph = gPackHookState.load(std::memory_order_acquire);
            auto hookLabel = [](int v) -> const char* {
                if (v == 1) return "OK";
                if (v == -1) return "BAD RVA";
                if (v == -2) return "HOOK FAIL";
                return "WAIT";
            };
            ImGui::TextDisabled("Hooks: Character %s | Role %s | Pack %s",
                                hookLabel(ch), hookLabel(rh), hookLabel(ph));
            const RuntimeJoinLayout layout = JoinLayoutSnapshot();
            ImGui::TextDisabled("Join: C@0x%lX -> ResAvatar@0x%lX = %zu",
                                (unsigned long)layout.characterItemAbs,
                                (unsigned long)layout.itemAvatarAbs,
                                layout.avatarJoinScore);
            ImGui::TextDisabled("IDs: Res@0x%lX -> Role@0x%lX = %zu | FullChain %zu | Names %zu",
                                (unsigned long)layout.itemIdAbs,
                                (unsigned long)layout.roleIdAbs,
                                layout.roleJoinScore, layout.chainScore, layout.nameScore);
            if (!gHookInstallOk.load(std::memory_order_acquire))
                ImGui::TextDisabled("Capture hooks are still arming in the background.");
        }
    }
    ImGui::EndChild();

    DrawDetachedSearchKeyboard("##CHARACTER_SKIN_KEYBOARD", "Search Skin",
                               gSkinSearch, sizeof(gSkinSearch), &gSkinKeyboard);
    DrawDetachedSearchKeyboard("##CHARACTER_TARGET_KEYBOARD", "Search Custom Char",
                               gTargetSearch, sizeof(gTargetSearch), &gTargetKeyboard);
}

} // namespace OblivionCharacterSkin
