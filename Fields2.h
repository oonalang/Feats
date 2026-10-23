#pragma once

struct RoleSkinFields {
     uint8_t ClothType;
     uint8_t ColorID;
     uint8_t GoGetPos;
     uint8_t HatSubMode;
     uint8_t IsCanBeUsedForAllRole;
     bool IsCloseScreenSaver;
     uint8_t SecondTab;
     bool ShowInBag;
     bool ShowRare;
     int BRBagAssetID;
     int BRClothAssetID;
     int BRHatAssetID;
     int BRLeftArmAssetID;
     int BRSuitAssetID;
     int BRWingAssetID;
     int BagAssetID;
     int CamoID;
     int ClothAssetID;
     int FxAssetIDHandGestureUI;
     int FxAssetIDShow;
     int FxAssetID_1P;
     int FxAssetID_3P;
     int FxAssetID_UI;
     int HatAssetID;
     uint ID;
     int LeftArm1PAssetID;
     int LeftArmAssetID;
     int ShowBagAssetID;
     int ShowClothAssetID;
     int ShowHatAssetID;
     int SuitAssetID;
     int WingAssetID;
};
RoleSkinFields* roleskinFields;

struct VehicleSkinConfFields {
    uint8_t ColorID;
    bool ExternalUnVisible;
    uint8_t GoGetPos;
    bool IsAutoDemolition;
    uint8_t LockedShowOrder;
    bool ShowInBag;
    bool ShowRare;
    uint8_t SkinMusicsID;
    uint8_t SkinStyle;
    char pad_19[3];
    uint CurrencyID;
    uint ID;
    int MusicsID;
    int NewVehicleResId;
    float RoomShowScale;
    int SellPrice;
    int SkinExtraMesh;
    int SkinParam;
    int SkinParam2;
    int SkinParam3;
    int UILowModel;
    int VehicleId;
};
VehicleSkinConfFields* vehicleSkinFields;

struct CharacterModelFields {
     bool Female;
     uint AvatarAnimationType;
     int BRBagModel;
     int BRHeadModel;
     int BRLobby;
     int BRModel;
     int BindEffect1P;
     int ChangeClipEffect1P;
     int DefaultModelID;
     int Guarder1P;
     int Guarder3P;
     int GuarderBagModel;
     int GuarderHeadModel;
     int GuarderLobby;
     int HeadIgnoreGUPSkin;
     int SpReceiveAnimType;
     int Traitor1P;
     int Traitor3P;
     float* BagOffset;
     string* Desc;
     float* VehicleOffset;
     int* VestOtherModelID;
     uintptr_t ItemID;
     uintptr_t LitePackDefault;
};
CharacterModelFields* characterfields;

struct RolePackFields {
     int mMatchRoomEasterEggType;
     uint mMatchRoomEasterEggCharacterID;
     String * mMatchRoomEasterEggActionName;
     bool EntryDefaultAppearance;
     bool UseVFWFeature;
     int AirItem1;
     int AnimationWhenSell;
     int CabinIndivID;
     int DeadBoxID;
     int DynamicRoomAssetID;
     int EntryAnimID;
     int EntryUIEffectAssetID;
     int GestureId;
     int HandEffectUI;
     int KillStreakSkinID;
     int LoadingFrame;
     int LobbySceneType;
     int MVPIndivID;
     int MatchRoomSort;
     int OpenSceneIndivID;
     int PendantShowID;
     int PetID;
     int PetSwitchEffectAssetID;
     uint RolePackID;
     int SkillSkinID;
     int SkillSkinUI;
     int SpecialSceneFrameGroupAssetID;
     int VTOLShowAssetID;
     String * AirPoseBR;
     String * EntryAnim;
     String * EntryIdleAnim;
     void* EntryIdleItems;
     void* EntryItems;
     void* EntryItemsAnim;
     void* EntryItemsIdleAnim;
     String * EntryUIAnimationStateName;
};
RolePackFields* packfields;

struct RoleConfFields {
    uint BaseCharacterID;
    uint roleLeftArmID;
    uint roleFinalSuitID;
    uint roleBasicHologramID;
    uint8_t ArchiveSwitch;
    uint8_t ArmorType;
    bool    BlockBack;
    uint8_t ColorID;
    uint8_t ColorSubID;
    uint8_t DefaultPerk_1;
    uint8_t DefaultPerk_2;
    uint8_t DefaultPerk_3;
    bool    DefaultShow;
    uint8_t DefaultSkill_1;
    bool    ExternalUnVisible;
    bool    IsVFW;
    uint8_t Loadout_Magnifier_Flag;
    uint8_t RoleLvGroupID;
    bool    ShowInBag;
    uint8_t ShowOrder;
    bool    ShowRare;
    uint    ID;
    int     JetPackInfoID;
    int     KeepInCatalog;
    float   RTPCMaxValue;
    int     RolePackID;
    void *AudioSwitch;
    void *BRBankNameEX;
    void *BasicAudioBank;
    void *BasicAudioBank2;
    void *BasicAudioBank3;
    void *DefaultAudioBank;
    void *DefaultAudioBank2;
    void *DefaultAudioEventEx;
    void *FoleyAudioBank;
    void *FoleyAudioNameEx;
    void *LOCID_Desc;
    void *LOCID_Name;
    void *LitePackageKeyName;
    void *MPBankNameEx;
    void *PVEBankNameEX;
    void *RTPCName;
    void *RoleSkinBox;
    void *SpecialAudioBank;
    void *SpecialAudioBank2;
    void *SpecialAudioEventEx;
    void *SpecialTips;
    uint64_t WinnerCircle;
};
RoleConfFields* roleFields;

struct ItemResourceFields {
     int FxAssetID;
     int ID;
     int InventoryModelID;
     int ModelAssetIDRaw;
     void *UIMiniSpriteName;
     void *UISmallSpriteName;
     void *UISpriteName;
     void *UISquareSpriteName;
     uintptr_t AvatarModelID;
};
ItemResourceFields* itemFields;

struct WeaponConfFields {
     uint8_t Accuracy;
     bool CanEquipOptic;
     uint8_t ColorID;
     uint8_t ColorSubID;
     uint8_t Control;
     uint8_t Damage;
     bool ExternalUnVisible;
     uint8_t FireRate;
     uint8_t LOCID_HowToGet;
     uint8_t MaxAttach;
     uint8_t MaxSkill;
     uint8_t Mobility;
     uint8_t Range;
     bool ShowRare;
     uint8_t SuperShowGroup;
     uint8_t WeaponBluePrintType;
     uint8_t WinnerCircleAnimID;
     uint BaseWeaponId;
     int DefWeaponSkinID;
     int DefaultKillBrocast;
     uint ID;
     int WinnerCircleWeaponMountType;
     void *EnableCamoBeta;
     void *LOCID_Desc;
     void *LOCID_Name;
     void *ModelRotation;
     void *UIProperty;
     void *UIPropertyUnlockLevel;
};
WeaponConfFields* weaponconfFields;

struct Item2InventoryFields {
    int ItemID;
    int WeaponAssetGroupID;
    int WeaponIconID;
    void *AssetName;
    void *ConfigFilePathH;
    void *ConfigFilePath_BRH;
    void *ConfigFilePath_DMZH;
    void *ConfigFilePath_PVEH;
    void *WeaponNameH;
};
Item2InventoryFields* item2Fields;

struct WeaponAssetGroupFields {
    int FireEffectGroupID;
    int Id;
    int MeshAssetID1P;
    int MeshAssetID3P;
    int MeshAssetID3P_L;
    int MeshAssetIDUI;
    int ProjAssetID;
    int ProjExplosionAssetID;
    int ProjFlightAssetID;
    int ProjFlightAssetID3P;
    int SecendBRProjExplosionAssetID;
    int SecendProjAssetID;
    int SecendProjExplosionAssetID;
    int SecendProjFlightAssetID;
    int SecendProjFlightAssetID3P;
    int SecondaryFireEffectGroupID;
    int SpecialUnAimingEffectAssetID1P;
    int UnusePupil;
    int WeaponPickUpAssetID;
    void *LeftLaserRotation;
    void *RightLaserRotation;
    void *ScopeDistancePartIDs;
    void *ScopeDistanceRatios;
};
WeaponAssetGroupFields* weaponAssetFields;

struct WeaponFireEffectFields {
    int AssetIdFor1P;
    int AssetIdFor3P;
    int AssetIdForBulletSmoke1P;
    int AssetIdForBulletSmoke3P;
    int AssetIdForBulletTrack1P;
    int AssetIdForBulletTrack3P;
    int AssetIdForCharging1P;
    int AssetIdForCrossHair;
    int AssetIdForDecalEffect;
    int AssetIdForExtra1P;
    int AssetIdForExtraBulletSmoke1P;
    int AssetIdForExtraBulletSmoke3P;
    int AssetIdForHip1P;
    int AssetIdForHip3P;
    int AssetIdForHitEnemy;
    int AssetIdForInspectionScreenEffect;
    int AssetIdForSecondBulletSmoke1P;
    int AssetIdForSecondBulletSmoke3P;
    int AssetIdForSecondHip1P;
    int AssetIdForSecondHip3P;
    int AssetIdForShellDrop1P;
    int AssetIdForShellDrop3P;
    int AttachAssetIdFor1P;
    int AttachAssetIdFor3P;
    int AttachAssetIdForHip1P;
    int AttachAssetIdForHip3P;
    int DefaultKillEffectLevelUI;
    float HitEffectDelayDestroyTime;
    int HitEffectType;
    int Id;
    float LevelEffectDelayTimeUI;
    float LevelEffectDelayTimeUI_King;
    void *AssetIdForInspectionEffect;
    void *AssetIdForLevelEffects;
    void *AssetIdForLevelEffectsUI;
    void *AssetIdsForCircleBluntDecal;
    void *AssetIdsForCrustDecal;
    void *AssetIdsForExtraEffects;
    void *AssetIdsForExtraEffects3P;
    void *AssetIdsForExtraEffectsUI;
    void *AssetIdsForMeleeAttack;
    void *AssetIdsForMetalDecal;
    void *AssetIdsForShellDrop1P;
    void *AssetIdsForShellDrop3P;
    void *AssetIdsForStripBluntDecal;
    void *BRKillCountTimePlus;
    void *BRKillCountTimePlus_King;
    void *FireSoundExtend;
    void *HitEffectAssetIds;
    void *KillCountForLevelEffect;
    void *KillCountForLevelEffect_King;
    void *MPKillCountTimePlus;
    void *MPKillCountTimePlus_King;
    void *MeleeEffectDetachParent;
    void *SocketForInspectionEffect;
    void *SocketForLevelEffects;
    void *SocketForLevelEffectsUI;
    void *SocketNameForExtra1P;
    void *SocketsForExtraEffects;
    void *SocketsForExtraEffects3P;
    void *SocketsForExtraEffectsUI;
    void *SocketsForMeleeAttack;
    void *SocketsForShellDrop1P;
    void *SocketsForShellDrop3P;
};
WeaponFireEffectFields* weaponfireFields;

struct WeaponConfExtraFields {
    uint8_t DefaultMythicArmor;
    uint8_t DefaultMythicSig;
    uint8_t GoGetPos;
    uint8_t SecondTab;
    uint8_t ThirdTab;
    int DefaultDeadReplayEffectId;
    int DefaultKillEffectId;
    int FireEffectGroupID;
    uint ID;
    void *GoGetUrl;
    void *IdleAnimationIndex;
    void *OpenViewSkill;
    void *OpenViewSkillUnlockLevel;
    void *PropertyModifys;
};
WeaponConfExtraFields* weaponextraFields;

struct MythicArmorFields {
    int AssetID;
    int ColorID;
    int ColorSubID;
    int GoGetPos;
    uint ID;
    int MythicArmorLevel;
    int SecondTab;
    int ThirdTab;
    void *EquippedWeaponIcon;
    void *GoGetUrl;
    void *LOCID_Desc;
    void *LOCID_HowToGet;
    void *LOCID_Name;
    void *WeaponCanEquip;
};
MythicArmorFields* mythicarmorFields;

struct MythicSightFields {
     bool ShowInBag;
     char pad_11[3];
     int AssetID;
     int ColorID;
     int ColorSubID;
     uint ID;
     int RelateMythicSight;
     void* LOCID_Desc;
     void* LOCID_Name;
     Array<int> **WeaponCanEquip;
};
MythicSightFields* mythicsightFields;

struct KillEffectItemFields {
     Array<int> **RealAssetIDs;
     bool IsAutoDemolition;
     bool ShowInBag;
     int ColorID;
     int ColorSubID;
     uint CurrencyID;
     int GoGetPos;
     uint ID;
     int SecondTab;
     int SellPrice;
     int SellProtection;
     int ThirdTab;
     Array<int> **AssetID;
     Array<int> **AssetID_CN;
     Array<int> **AssetID_GA;
     Array<int> **AssetID_KR;
     Array<int> **AssetID_VNG;
     string* GoGetUrl;
     Array<int> **KillCount;
     void* LOCID_Desc;
     void* LOCID_HowToGet;
     void* LOCID_Name;
     Array<int> **WeaponCanEquip;
};
KillEffectItemFields* killeffectFields;

struct WeaponCamosFields {
    uint8_t CamoType;
    uint8_t ColorID;
    bool NeedShowInEndGame;
    int BelongWeaponID;
    uint CamoItemID;
    uint ConceptID;
    int DMZseason;
    int KeepInCatalog;
    int TextureAssetID;
    int TextureSpecialAssetID;
    void *LOCID_Describe;
    void *LOCID_Name;
    Array<uint> *Mission_id;
    Array<uint> *Personal_mission_id;
    void *SmallIconSpriteName;
};
WeaponCamosFields* weaponcamosFields;

struct WeaponSkinFields {
    uint8_t ColorID;
    uint8_t GoGetPos;
    bool IsCleanCamo;
    uint8_t SkinType;
    uint BRWeaponID;
    int BrSkinWeaponID;
    int CamoID;
    int ColorSubID;
    uint ID;
    uint WeaponID;
    void *LOCID_Describe;
    void *LOCID_HowToGet;
    void *LOCID_Name;
    Array<int> *SkinID1P;
    Array<int> *SkinID3P;
};
WeaponSkinFields* weaponskinFields;

struct BRDeadboxSkinFields {
    int ColorID;
    int DeadBoxEffectAsset;
    int Flag;
    int FlagAsset;
    uint ID;
    int ModelAsset3P;
    int ModelAssetUI;
};
BRDeadboxSkinFields* deadboxFields;

struct BRDropPlaneSkinFields {
     int ColorID;
     uint ID;
     int ModelAsset1P;
     int ModelAsset3P;
     int ModelAssetCutScene;
     int ModelAssetUI;
     int Priority;
};
BRDropPlaneSkinFields* dropplaneFields;

static char searchQuery[256] = "";

bool loadskinhack = false;
bool loadCharacter = false;
bool isLoad = false;

std::string lastKnownName = "";
int emptyNameCount = 0;
int g_selectedTargetCharIndex = 0;
void* lastKnownLocId = nullptr;

struct itemInfo {
    std::string itemName;
    std::array<int, 4> Item2Inventory;
    std::array<int, 5> WeaponConf;
    std::array<int, 5> WeaponExtra;
    std::array<int, 3> WeaponAsset;
    // Fire-effect attachment assets used by the original SkinHax path.
    std::array<int, 4> WeaponAttach;
    // Per-attachment-type mythic appearances supplied by the updated attachment engine.
    std::array<int, 31> MythicAttach;
    std::array<int, 3> ItemResInt;
    std::array<void *, 4> ItemRes;
};
std::vector<itemInfo> itemData;

std::unordered_map<std::string, int> nameCountMap;
std::unordered_map<std::string, int> nameCountChar;

struct charInfo {
    std::string charName;
    std::array<int, 12> charModel;
    std::array<int, 3> charRes;
    std::array<void *, 4> charRes2;
    std::array<int, 8> charRole;
    std::array<void *, 1> charRole2;
    std::array<int, 9> charPack;
};

std::vector<charInfo> charData;

struct TargetChar {
    std::string name;
    int traitor1p;
    int traitor3p;
    int itemID;
    int roleID;
    int rolepackID;
};

std::vector<TargetChar> g_targetCharacters;

struct watcher {
    std::string watchname;
    int watchvalue;
};
std::vector<watcher> watch;

struct deadbox {
    std::string deadname;
    std::array<int, 6> dead;
};
std::vector<deadbox> deadboxF;

struct planeID {
    std::string planename;
    std::array<int, 6> plane;
};
std::vector<planeID> dropplane;

uintptr_t location = 0;

uintptr_t getRealOffset(uintptr_t offset) {
    while (location <= 0) {
        location = Tools::GetBaseAddress("libunity.so");
        if (location <= 0) {
            usleep(1000); 
        }
    }
    return location + offset;
}

std::string GetNameString(uintptr_t off, void *getadd) {
    auto getC = (String *(*)(void *)) (getRealOffset(off));
    if (Tools::IsPtrValid((void *) getC)) {
    auto getV = getC(getadd);
    if (!getV || !Tools::IsPtrValid((void*)getV)) return "";
    const char* text = getV->CString();
    return text ? std::string(text) : std::string();
    }
    return "";
}

std::string ToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

std::string GetBaseWeaponName(const std::string& fullName)
{
    size_t dashPos = fullName.find(" - ");
    if (dashPos == std::string::npos)
        return fullName;
    std::string base = fullName.substr(0, dashPos);
    size_t tagEnd = base.find("] ");
    if (tagEnd != std::string::npos && tagEnd + 2 < base.length())
        base = base.substr(tagEnd + 2);
    return base;
}

void KeyboardWindow(char* s, size_t capacity, bool h) {
    static bool useLowercase = true;
    if (!h || !s || capacity == 0) return;

    auto appendTextSafe = [&](const char* text) {
        const size_t len = strlen(s);
        const size_t add = strlen(text);
        if (len + add + 1 <= capacity) {
            strncat(s, text, capacity - len - 1);
        }
    };

    // Compact layout so every row stays visible on landscape phones.
    const ImVec2 contentSize = ImGui::GetContentRegionAvail();
    const float padding = 3.0f;
    const float buttonHeight = 25.0f;
    const float buttonWidth = (contentSize.x - (padding * 9.0f)) / 10.0f;

    const std::vector<std::vector<char>> keyboardLayoutLowercase = {
        {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
        {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
        {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'},
        {'z', 'x', 'c', 'v', 'b', 'n', 'm'},
    };

    const std::vector<std::vector<char>> keyboardLayoutUppercase = {
        {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
        {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
        {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'},
        {'Z', 'X', 'C', 'V', 'B', 'N', 'M'},
    };

    const std::vector<std::vector<char>>& keyboardLayout =
        useLowercase ? keyboardLayoutLowercase : keyboardLayoutUppercase;

    for (const auto& row : keyboardLayout) {
        for (size_t keyIndex = 0; keyIndex < row.size(); ++keyIndex) {
            const char c = row[keyIndex];
            char label[2] = { c, '\0' };
            ImGui::PushID((int)(c + keyIndex * 97));
            if (ImGui::Button(label, ImVec2(buttonWidth, buttonHeight))) {
                char add[2] = { c, '\0' };
                appendTextSafe(add);
            }
            ImGui::PopID();
            if (keyIndex + 1 < row.size()) ImGui::SameLine(0, padding);
        }
    }

    // Keep rarity shortcuts and the two most-used actions on one compact row.
    const char* tags[] = { "[M]", "[L]", "[E]", "[C]" };
    for (int t = 0; t < 4; ++t) {
        if (ImGui::Button(tags[t], ImVec2(buttonWidth, buttonHeight))) appendTextSafe(tags[t]);
        ImGui::SameLine(0, padding);
    }

    if (ImGui::Button(DEOBF("SPACE"), ImVec2(buttonWidth * 3.0f + padding * 2.0f, buttonHeight))) {
        appendTextSafe(" ");
    }
    ImGui::SameLine(0, padding);

    if (ImGui::Button(useLowercase ? DEOBF("SHIFT") : DEOBF("shift"),
                      ImVec2(buttonWidth * 2.0f + padding, buttonHeight))) {
        useLowercase = !useLowercase;
    }
}


// Floating keyboard used by search pages.  It is intentionally detached from the
// tab child region so the operator/skin list is never squeezed or clipped and
// every keyboard row remains visible on 16:9 / 20:9 landscape displays.
void DrawDetachedSearchKeyboard(const char* windowId, const char* title, char* buffer,
                                size_t capacity, bool* openState) {
    if (!openState || !*openState || !buffer || capacity == 0) return;

    ImGuiIO& kbIO = ImGui::GetIO();
    float kbW = kbIO.DisplaySize.x * 0.60f;
    if (kbW < 700.0f) kbW = 700.0f;
    if (kbW > 920.0f) kbW = 920.0f;
    if (kbW > kbIO.DisplaySize.x - 20.0f) kbW = kbIO.DisplaySize.x - 20.0f;

    // Compact enough to display every key row, while still being detached.
    float kbH = 238.0f;
    if (kbH > kbIO.DisplaySize.y - 16.0f) kbH = kbIO.DisplaySize.y - 16.0f;

    const ImVec2 kbPos((kbIO.DisplaySize.x - kbW) * 0.5f,
                       kbIO.DisplaySize.y - kbH - 8.0f);
    ImGui::SetNextWindowPos(kbPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(kbW, kbH), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.0f, 3.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.030f, 0.028f, 0.038f, 0.990f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.40f, 0.20f, 0.58f, 1.0f));

    const ImGuiWindowFlags kbFlags = ImGuiWindowFlags_NoTitleBar |
                                     ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoScrollWithMouse;

    if (ImGui::Begin(windowId, openState, kbFlags)) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.72f, 0.43f, 0.95f, 1.0f));
        ImGui::TextUnformatted(title ? title : DEOBF("SEARCH KEYBOARD"));
        ImGui::PopStyleColor();

        // DELETE / CLEAR are kept in the fixed header so they can never disappear
        // below the phone's visible area.
        const float actionW = 72.0f;
        const float gap = 4.0f;
        const float totalActionsW = actionW * 3.0f + gap * 2.0f;
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalActionsW - 10.0f);

        if (ImGui::Button(DEOBF("DELETE"), ImVec2(actionW, 25.0f))) {
            const size_t len = strlen(buffer);
            if (len > 0) buffer[len - 1] = '\0';
        }
        ImGui::SameLine(0, gap);
        if (ImGui::Button(DEOBF("CLEAR"), ImVec2(actionW, 25.0f))) {
            buffer[0] = '\0';
        }
        ImGui::SameLine(0, gap);
        if (ImGui::Button(DEOBF("HIDE"), ImVec2(actionW, 25.0f))) {
            *openState = false;
        }

        ImGui::Separator();
        KeyboardWindow(buffer, capacity, true);
    }
    ImGui::End();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(4);
}
