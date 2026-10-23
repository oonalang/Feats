
#pragma once

static ESP * m_Canvas = 0;
#include <malloc.h>
#include <errno.h>
#include <stdarg.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstddef>

// Readable mapped-memory checks used only by the ESP renderer. This is not a
// universal stale-pointer guarantee, but it prevents obvious unmapped reads.
struct StretchESPMapRegion {
    uintptr_t begin;
    uintptr_t end;
    bool readable;
};

static bool StretchESPIsReadable(const void* pointer, size_t bytes) {
    if (!pointer || bytes == 0) return false;
    const uintptr_t begin = reinterpret_cast<uintptr_t>(pointer);
    if (begin > UINTPTR_MAX - bytes) return false;
    const uintptr_t end = begin + bytes;

    static std::vector<StretchESPMapRegion> regions;
    static double lastRefresh = -1000.0;
    const double now = ImGui::GetTime();
    if (regions.empty() || now - lastRefresh >= 0.50) {
        std::vector<StretchESPMapRegion> refreshed;
        FILE* maps = std::fopen("/proc/self/maps", "r");
        if (maps) {
            char line[512];
            while (std::fgets(line, sizeof(line), maps)) {
                unsigned long long low = 0, high = 0;
                char permissions[5] = {0};
                if (std::sscanf(line, "%llx-%llx %4s", &low, &high, permissions) == 3 && high > low) {
                    StretchESPMapRegion region;
                    region.begin = static_cast<uintptr_t>(low);
                    region.end = static_cast<uintptr_t>(high);
                    region.readable = permissions[0] == 'r';
                    refreshed.push_back(region);
                }
            }
            std::fclose(maps);
        }
        if (!refreshed.empty()) regions.swap(refreshed);
        lastRefresh = now;
    }

    if (regions.empty()) return false;
    auto it = std::upper_bound(regions.begin(), regions.end(), begin,
        [](uintptr_t value, const StretchESPMapRegion& region) { return value < region.begin; });
    if (it == regions.begin()) return false;
    --it;
    return it->readable && begin >= it->begin && end <= it->end;
}

template <typename T>
static bool StretchESPRead(uintptr_t address, T& value) {
    if (!StretchESPIsReadable(reinterpret_cast<const void*>(address), sizeof(T))) return false;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(T));
    return true;
}

static bool StretchESPFinite3(const Vector3& value) {
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

static bool StretchESPCopyString(String* value, char* output, size_t capacity) {
    if (!output || capacity == 0) return false;
    output[0] = '\0';
    if (!StretchESPIsReadable(value, offsetof(String, start_char) + sizeof(uint16_t))) return false;
    int32_t length = 0;
    if (!StretchESPRead((uintptr_t)value + offsetof(String, length), length) || length < 0 || length > 90) return false;
    const uintptr_t charsAddress = (uintptr_t)value + offsetof(String, start_char);
    if (!StretchESPIsReadable(reinterpret_cast<const void*>(charsAddress), (size_t)length * sizeof(uint16_t))) return false;
    const uint16_t* chars = reinterpret_cast<const uint16_t*>(charsAddress);
    size_t out = 0;
    for (int i = 0; i < length; ++i) {
        uint16_t unit = 0;
        if (!StretchESPRead((uintptr_t)(chars + i), unit)) return false;
        // Avoid Il2CppString::CString(): this project returns c_str() from a temporary std::string.
        uint32_t cp = unit;
        if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < length) {
            uint16_t low = chars[i + 1];
            if (low >= 0xDC00 && low <= 0xDFFF) {
                cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                ++i;
            } else cp = '?';
        } else if (cp >= 0xDC00 && cp <= 0xDFFF) cp = '?';

        char encoded[4]; size_t count = 0;
        if (cp <= 0x7F) { encoded[0] = (char)cp; count = 1; }
        else if (cp <= 0x7FF) {
            encoded[0] = (char)(0xC0 | (cp >> 6));
            encoded[1] = (char)(0x80 | (cp & 0x3F)); count = 2;
        } else if (cp <= 0xFFFF) {
            encoded[0] = (char)(0xE0 | (cp >> 12));
            encoded[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
            encoded[2] = (char)(0x80 | (cp & 0x3F)); count = 3;
        } else {
            encoded[0] = (char)(0xF0 | (cp >> 18));
            encoded[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
            encoded[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
            encoded[3] = (char)(0x80 | (cp & 0x3F)); count = 4;
        }
        if (out + count >= capacity) break;
        std::memcpy(output + out, encoded, count);
        out += count;
    }
    output[out] = '\0';
    return out > 0;
}
char extra[30];
int atas, bawah, kiri, kanan;
int32_t ToColor(float * col) {
    return ImGui::ColorConvertFloat4ToU32( * (ImVec4 *)(col));
}

#define WHITE               ARGB(255, 255, 255, 255)
#define RED                 ARGB(255, 0, 0, 255)
#define GREEN               ARGB(255, 000, 128, 000)
#define LIME                ARGB(255, 000, 255, 000)
#define BLUE                ARGB(255, 000, 000, 255)
#define BLACK               ARGB(255, 000, 000, 000)
#define PURPLE              ARGB(255, 125, 000, 255)
#define GREY                ARGB(255, 128, 128, 128)
#define YELLOW              ARGB(255, 255, 0, 255)
#define ORANGE              ARGB(255, 255, 125, 000)
#define DARK_GREEN          ARGB(255, 000, 100, 000)
#define PINK                ARGB(255, 255, 192, 203)
#define BROWN               ARGB(255, 210, 105, 30)
#define CYAN                ARGB(255, 000, 255, 255)
#define BlueDongker         ARGB(255, 000, 000, 102)
#define SilverDark          ARGB(255, 064, 064, 064)
#define Magenta             ARGB(255, 255, 000, 255)
#define CLEAR               ARGB(001, 000, 000, 000)
#define GREENA             ARGB(52, 235, 183, 255)
#define GREENB             ARGB(201, 52, 235, 255)
#define GREENC             ARGB(235, 207, 52, 255)
#define GREEND             ARGB(204, 235, 52, 255)
#define GREENE             ARGB(135, 227, 23, 255)
#define GREENF             ARGB(17, 214, 240, 255)
#define GREENG             ARGB(20, 227, 120, 255)
#define GREENH             ARGB(101, 127, 199, 255)
#define GREENI             ARGB(109, 158, 25, 255)
#define GREENJ             ARGB(194, 27, 122, 255)
#define GREENK             ARGB(255, 10, 10, 255)
#define GREENL             ARGB(219, 104, 22, 255)

ImU32 teamIdColors[] =
{
        RED, // 0
        YELLOW, // 1
        ORANGE, // 2
        BlueDongker, // 3
        PINK, // 4
        GREEN, // 5
        GREY, //6
        SilverDark, // 7
        BLUE, // 8
        CYAN, // 9
        PURPLE, // 10
        Magenta, // 11
        BROWN, // 12
        GREENA,
        GREENB,
        GREENC,
        GREEND,
        GREENE,
        GREENF,
        GREENG,
        GREENH,
        GREENI,
        GREENJ,
        GREENK,
        GREENL,
};

int totalBots, totalEnemies;
bool isEspReady;

Vector2 pushToScreenBorder(Vector2 Pos, Vector2 screen, int borders, int offset) {
    int x = (int)Pos.x;
    int y = (int)Pos.y;
    if ((borders & 1) == 1) {
        y = 0 - offset;
    }
    if ((borders & 2) == 2) {
        x = (int)screen.x + offset;
    }
    if ((borders & 4) == 4) {
        y = (int)screen.y + offset;
    }
    if ((borders & 8) == 8) {
        x = 0 - offset;
    }
    return Vector2(x, y);
}
int isOutsideSafezone(Vector2 pos, Vector2 screen) {
    Vector2 mSafezoneTopLeft(screen.x * 0.04f, screen.y * 0.04f);
    Vector2 mSafezoneBottomRight(screen.x * 0.96f, screen.y * 0.96f);

    int result = 0;
    if (pos.y < mSafezoneTopLeft.y) {
        result |= 1;
    }
    if (pos.x > mSafezoneBottomRight.x) {
        result |= 2;
    }
    if (pos.y > mSafezoneBottomRight.y) {
        result |= 4;
    }
    if (pos.x < mSafezoneTopLeft.x) {
        result |= 8;
    }
    return result;
}

uintptr_t GetClosestTarget() {
    uintptr_t result = 0;
    float MaxDist = std::numeric_limits<float>::infinity();

        auto Gameplay_get_MatchGame = (uintptr_t (*)()) (Class_Gameplay_get_MatchGame);
        auto get_MatchGame = Gameplay_get_MatchGame();
        if (get_MatchGame) {
            auto Gameplay_get_LocalPawn = (uintptr_t (*)()) (Class_Gameplay_get_LocalPawn);
            auto LocalPawn = Gameplay_get_LocalPawn();
            if (LocalPawn) {
                Vector3 MyPos{0, 0, 0};

                auto local_m_Mesh = *(Transform **) (LocalPawn + Class_Pawn_m_Mesh);
                if (local_m_Mesh) {
                    MyPos = local_m_Mesh->get_position();
                }

                    auto EnemyPawns = *(List<uintptr_t> **) (get_MatchGame + Class_BaseGame_EnemyPawns);
                if (EnemyPawns) {
                    auto Items = EnemyPawns->getItems();
                    if (Items) {
                        for (int i = 0; i < EnemyPawns->getSize(); i++) {
                            auto Pawn = Items[i];
                            if (Pawn) {
                                
                                auto isAlive = *(bool*) ((uintptr_t)Pawn + Class_Pawn_m_IsAlive);
                                if (!isAlive)
                                    continue;
                                    

                                auto m_Mesh = *(Transform **) (Pawn + Class_Pawn_m_Mesh);
                                if (!m_Mesh)
                                    continue;
                                    
                                
                                
                               
                                auto RootPos = m_Mesh->get_position();
                                float Distance = Vector3::Distance(MyPos, RootPos);

                                if (Distance < MaxDist) {
                                    result = Pawn;
                                    MaxDist = Distance;
                                }
                            }
                        }
                    }
                }
            }
        }
    
    return result;
}



bool isInsideFOV(int x, int y) {
    if (!seekbarValues["FOV500"])
        return true;

    int circle_x = glWidth / 2;
    int circle_y = glHeight / 2;
    int rad = seekbarValues["FOV500"];
    return (x - circle_x) * (x - circle_x) + (y - circle_y) * (y - circle_y) <= rad * rad;
}

uintptr_t GetInsideFOVTarget() {
    uintptr_t result = 0;

    float MaxDist = std::numeric_limits<float>::infinity();

        auto Gameplay_get_MatchGame = (uintptr_t (*)()) (Class_Gameplay_get_MatchGame);
        auto get_MatchGame = Gameplay_get_MatchGame();
        if (get_MatchGame) {
            auto Gameplay_get_LocalPawn = (uintptr_t (*)()) (Class_Gameplay_get_LocalPawn);
            auto LocalPawn = Gameplay_get_LocalPawn();
            if (LocalPawn) {
                Vector3 MyPos{0, 0, 0};

                auto local_m_Mesh = *(Transform **) (LocalPawn + Class_Pawn_m_Mesh);
                if (local_m_Mesh) {
                    MyPos = local_m_Mesh->get_position();
                }

                auto EnemyPawns = *(List<uintptr_t> **) (get_MatchGame + Class_BaseGame_EnemyPawns);
                if (EnemyPawns) {
                    auto Items = EnemyPawns->getItems();
                    if (Items) {
                        for (int i = 0; i < EnemyPawns->getSize(); i++) {
                            auto Pawn = Items[i];
                            if (Pawn) {
                                            
                                auto isAlive = *(bool*) ((uintptr_t)Pawn + Class_Pawn_m_IsAlive);
                                if (!isAlive)
                                    continue;

                                auto m_HeadBone = *(Transform **) (Pawn + Class_Pawn_m_HeadBone);
                                if (!m_HeadBone)
                                    continue;
                                                                                                                                        
                                    
                                //auto HeadSc = WorldToScreenPoint(m_HeadBone->get_position());

                                auto HeadSc = Camera::get_main()->Camera::WorldToScreenPoint(m_HeadBone->get_position());
                                
                                
                                
                                Vector2 v2Middle = Vector2((float) (get_width() / 2), (float) (get_height() / 2));
                                Vector2 v2Loc = Vector2(HeadSc.x, HeadSc.y);

                                if (isInsideFOV((int) HeadSc.x, (int) HeadSc.y)) {
                                    float Distance = Vector2::Distance(v2Middle, v2Loc);

                                    if (Distance < MaxDist) {
                                        result = Pawn;
                                        MaxDist = Distance;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    return result;
}


void (*oWeaponFireComponent_Instant_CreateBulletLine)(uintptr_t thiz, Vector3 startPos, Vector3 dir, bool isDualFire);
void WeaponFireComponent_Instant_CreateBulletLine(uintptr_t thiz, Vector3 startPos, Vector3 dir, bool isDualFire) {
    //if (UNCRACKED == ENC) { 
    if (checkboxStates["BULLET TRACK 360"] || checkboxStates["AIMBOT 360"]) {
         auto Gameplay_get_MatchGame = (uintptr_t (*)()) (Class_Gameplay_get_MatchGame);
  auto get_MatchGame = Gameplay_get_MatchGame();
  if (get_MatchGame) {
   auto Gameplay_get_LocalPawn = (uintptr_t (*)()) (Class_Gameplay_get_LocalPawn);
                auto LocalPawn = Gameplay_get_LocalPawn();
    if (LocalPawn) {
        
     bool bTriggerReady = 1;
     if (dropdownStates["#TRIG"] == 1) {
      auto Pawn_get_IsFiring = (bool (*)(uintptr_t)) (Class_Pawn_get_IsFiring);
      bTriggerReady = Pawn_get_IsFiring(LocalPawn);
     } else if (dropdownStates["#TRIG"] == 2) {
      auto Pawn_IsAiming = (bool (*)(uintptr_t)) (Class_Pawn_IsAiming);
      bTriggerReady = Pawn_IsAiming(LocalPawn);
     } else if (dropdownStates["#TRIG"] == 3) {
      auto Pawn_get_TriggerQuickKnife = (bool (*)(uintptr_t)) (Class_Pawn_get_TriggerQuickKnife);
      bTriggerReady = Pawn_get_TriggerQuickKnife(LocalPawn);
     }
     
     
     
     if (bTriggerReady) {
      uintptr_t Target = 0;
      if (dropdownStates["#DIST"] == 0) {
       Target = GetClosestTarget();
      }
      if (dropdownStates["#DIST"] == 1) {
       Target = GetInsideFOVTarget();
      }
                        if (Target) {           
                            Vector3 targetPos;
                            if (dropdownStates["#TARG"] == 0) {  
                                    auto m_HeadBone = *(Transform **) (Target + Class_Pawn_m_HeadBone);
                                if (!m_HeadBone)
                                    return oWeaponFireComponent_Instant_CreateBulletLine(thiz, startPos, dir, isDualFire);
                                    targetPos = m_HeadBone->get_position();
                                    
                            }
                            if (dropdownStates["#TARG"] == 1) {  
                                    auto m_HeadBone = *(Transform **) (Target + Class_Pawn_m_HeadBone);
                                if (!m_HeadBone)
                                    return oWeaponFireComponent_Instant_CreateBulletLine(thiz, startPos, dir, isDualFire);
                                    targetPos = m_HeadBone->get_position();
                                    targetPos.y -= 0.2f;
                            }
                            if (dropdownStates["#TARG"] == 2) {  
                                    auto m_HeadBone = *(Transform **) (Target + Class_Pawn_m_HeadBone);
                                if (!m_HeadBone)
                                    return oWeaponFireComponent_Instant_CreateBulletLine(thiz, startPos, dir, isDualFire);
                                    targetPos = m_HeadBone->get_position();
                                    targetPos.y -= 0.4f;
                            }
                            
                            auto main = Camera::get_main();
                            if (main) {
                                auto mainView = ((Component *) main)->get_transform();
                                if (mainView) {
                                  if (checkboxStates["BULLET TRACK 360"]) {
                                    dir = targetPos - mainView->get_position();
                                  }
                                  if (checkboxStates["AIMBOT 360"]) {
                                   auto Pawn_set_AimRotation = (void (*)(uintptr_t, Quaternion)) (Class_Pawn_set_AimRotation);
                                   Pawn_set_AimRotation(LocalPawn, Quaternion::LookRotation(targetPos - mainView->get_position(), Vector3::Up()));
                                 
                                   }
                                    
                                }
                            }
                        }
                    }
                }
            }
        }
    
        return oWeaponFireComponent_Instant_CreateBulletLine(thiz, startPos, dir, isDualFire);
}


void (*oWeaponFireComponent_Instant_CreateBulletProjectile)(void* thiz, Vector3 startPos, Vector3 dir, void* weaponImpact, int itemID, int flySmokeAssetID, bool enableVirtualStartPos, Vector3 virtualStartPos);
void WeaponFireComponent_Instant_CreateBulletProjectile(void* thiz, Vector3 startPos, Vector3 dir, void* weaponImpact, int itemID, int flySmokeAssetID, bool enableVirtualStartPos, Vector3 virtualStartPos) {
   // if (UNCRACKED == ENC) { 
    if (checkboxStates["BULLET TRACK 360"] || checkboxStates["AIMBOT 360"]) {
         auto Gameplay_get_MatchGame = (uintptr_t (*)()) (Class_Gameplay_get_MatchGame);
  auto get_MatchGame = Gameplay_get_MatchGame();
  if (get_MatchGame) {
   auto Gameplay_get_LocalPawn = (uintptr_t (*)()) (Class_Gameplay_get_LocalPawn);
                auto LocalPawn = Gameplay_get_LocalPawn();
    if (LocalPawn) {
        
     bool bTriggerReady = 1;
     if (dropdownStates["#TRIG"] == 1) {
      auto Pawn_get_IsFiring = (bool (*)(uintptr_t)) (Class_Pawn_get_IsFiring);
      bTriggerReady = Pawn_get_IsFiring(LocalPawn);
     } else if (dropdownStates["#TRIG"] == 2) {
      auto Pawn_IsAiming = (bool (*)(uintptr_t)) (Class_Pawn_IsAiming);
      bTriggerReady = Pawn_IsAiming(LocalPawn);
     } else if (dropdownStates["#TRIG"] == 3) {
      auto Pawn_get_TriggerQuickKnife = (bool (*)(uintptr_t)) (Class_Pawn_get_TriggerQuickKnife);
      bTriggerReady = Pawn_get_TriggerQuickKnife(LocalPawn);
     }
     
     
     
     if (bTriggerReady) {
      uintptr_t Target = 0;
      if (dropdownStates["#DIST"] == 0) {
       Target = GetClosestTarget();
      }
      if (dropdownStates["#DIST"] == 1) {
       Target = GetInsideFOVTarget();
      }
      if (Target) {
       Vector3 targetPos;
       if (dropdownStates["#TARG"] == 0) {  
        auto m_HeadBone = *(Transform **) (Target + Class_Pawn_m_HeadBone);
       if (!m_HeadBone)
       return oWeaponFireComponent_Instant_CreateBulletProjectile(thiz, startPos, dir, weaponImpact, itemID, flySmokeAssetID, enableVirtualStartPos, virtualStartPos);
       targetPos = m_HeadBone->get_position();
       }
       if (dropdownStates["#TARG"] == 1) {  
            auto m_HeadBone = *(Transform **) (Target + Class_Pawn_m_HeadBone);
        if (!m_HeadBone)
           return oWeaponFireComponent_Instant_CreateBulletProjectile(thiz, startPos, dir, weaponImpact, itemID, flySmokeAssetID, enableVirtualStartPos, virtualStartPos);
           targetPos = m_HeadBone->get_position();
           targetPos.y -= 0.2f;
       }
       if (dropdownStates["#TARG"] == 2) {  
           auto m_HeadBone = *(Transform **) (Target + Class_Pawn_m_HeadBone);
       if (!m_HeadBone)
           return oWeaponFireComponent_Instant_CreateBulletProjectile(thiz, startPos, dir, weaponImpact, itemID, flySmokeAssetID, enableVirtualStartPos, virtualStartPos);
           targetPos = m_HeadBone->get_position();
           targetPos.y -= 0.4f;
       }
       
       
          
       
          auto main = Camera::get_main();
            if (main) {
              auto mainView = ((Component *) main)->get_transform();
                if (mainView) {
                   if (checkboxStates["BULLET TRACK 360"]) {
                       dir = targetPos - mainView->get_position();          
                   }
                   if (checkboxStates["AIMBOT 360"]) {
                       auto Pawn_set_AimRotation = (void (*)(uintptr_t, Quaternion)) (Class_Pawn_set_AimRotation);
                       Pawn_set_AimRotation(LocalPawn, Quaternion::LookRotation(targetPos - mainView->get_position(), Vector3::Up()));
                         
                   }
   
        }
       }
       
       
      }
     }
    }
   }
  } 
  
       return oWeaponFireComponent_Instant_CreateBulletProjectile(thiz, startPos, dir, weaponImpact, itemID, flySmokeAssetID, enableVirtualStartPos, virtualStartPos);

}

bool isOutsideScreen(ImVec2 pos, ImVec2 screen) {
    if (pos.y < 0) {
        return true;
    }
    if (pos.x > screen.x) {
        return true;
    }
    if (pos.y > screen.y) {
        return true;
    }
    return pos.x < 0;
}

ImVec2 pushToScreenBorder(ImVec2 Pos, ImVec2 screen, int offset) {
    int x = (int)Pos.x;
    int y = (int)Pos.y;
    if (Pos.y < 0) {
        // top
        y = -offset;
    }
    if (Pos.x > screen.x) {
        // right
        x = (int)screen.x + offset;
    }
    if (Pos.y > screen.y) {
        // bottom
        y = (int)screen.y + offset;
    }
    if (Pos.x < 0) {
        // left
        x = -offset;
    }
    return ImVec2(x, y);
}

void drawLine(long color, float thickness, Vector3 start, Vector3 end) {
    ImGui::GetBackgroundDrawList()->PathLineTo(ImVec2(start.x, start.y));
    ImGui::GetBackgroundDrawList()->PathLineTo(ImVec2(end.x, end.y));
    ImGui::GetBackgroundDrawList()->PathStroke(color, 0, thickness);
}

void DrawBoxEnemy(ImVec2 X, ImVec2 Y, float thicc, int color) {
    drawLine(color, thicc, {X.x, X.y}, {Y.x, Y.y});
}

void DrawBoxEnemy(ImDrawList *draw, ImVec2 X, ImVec2 Y, float thicc, int color) {
    draw->AddLine({X.x, X.y}, {Y.x, Y.y}, color, thicc);
}

void Box4Line(ImDrawList * draw, float thicc, int x, int y, int w, int h, int color) {
        int iw = w / 4;
        int ih = h / 4;
        // top
        draw->AddRect(ImVec2(x, y), ImVec2(x + iw, y), color, thicc);
        draw->AddRect(ImVec2(x + w - iw, y), ImVec2(x + w, y), color, thicc);
        draw->AddRect(ImVec2(x, y), ImVec2(x, y + ih), color, thicc);
        draw->AddRect(ImVec2(x + w - 1, y), ImVec2(x + w - 1, y + ih), color, thicc);
    ;
        // bottom
        draw->AddRect(ImVec2(x, y + h), ImVec2(x + iw, y + h), color, thicc);
        draw->AddRect(ImVec2(x + w - iw, y + h), ImVec2(x + w, y + h), color, thicc);
        draw->AddRect(ImVec2(x, y + h - ih), ImVec2(x, y + h), color, thicc);
        draw->AddRect(ImVec2(x + w - 1, y + h - ih), ImVec2(x + w - 1, y + h), color, thicc);
}

void DrawText(ImDrawList *draw, const std::string &text, const Vector2 &position, ImU32 color, float fontSize)
{
    draw->AddText(NULL, fontSize, {position.x, position.y}, color, text.c_str());
}

void DrawTextWithBorder(ImDrawList *draw, const std::string &text, const Vector2 &position, ImU32 textColor, ImU32 borderColor, float fontSize)
{
    float borderSize = 1.0f;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            if (x == 0 && y == 0)
                continue;

            DrawText(draw, text, {position.x + x * borderSize, position.y + y * borderSize}, borderColor, fontSize);
        }
    }
    DrawText(draw, text, position, textColor, fontSize);
}

#define IM_PI                   3.14159265358979323846f
#define RAD2DEG( x )  ( (float)(x) * (float)(180.f / IM_PI) )
#define DEG2RAD( x ) ( (float)(x) * (float)(IM_PI / 180.f) )

void RotateTriangle(std::array<Vector3, 3> & points, float rotation) {
const auto points_center = (points.at(0) + points.at(1) + points.at(2)) / 3;
for (auto & point : points) {
point = point - points_center;
const auto temp_x = point.x;
const auto temp_y = point.y;
const auto theta = DEG2RAD(rotation);
const auto c = cosf(theta);
const auto s = sinf(theta);
point.x = temp_x * c - temp_y * s;
point.y = temp_x * s + temp_y * c;
point = point + points_center;}}

void VectorAnglesRadar(Vector3 & forward, Vector3 & angles) {
if (forward.x == 0.f && forward.y == 0.f) {
angles.x = forward.z > 0.f ? -90.f : 90.f;
angles.y = 0.f;
} else {
angles.x = RAD2DEG(atan2(-forward.z, forward.Magnitude(forward)));
angles.y = RAD2DEG(atan2(forward.y, forward.x));
}angles.z = 0.f;}


void IMGUI_GOD(float *angles) {
    if (angles[0] > 180)
        angles[0] -= 360;
    if (angles[0] < -180)
        angles[0] += 360;
    if (angles[0] < -75.f)
        angles[0] = -75.f;
    else if (angles[0] > 75.f)
        angles[0] = 75.f;
    while (angles[1] < -180.0f)
        angles[1] += 360.0f;
    while (angles[1] > 180.0f)
        angles[1] -= 360.0f;
}
void IMGUI_GOD(Vector3 angles) {
    if (angles.x > 180)
        angles.x -= 360;
    if (angles.x < -180)
        angles.x += 360;
    if (angles.x < -75.f)
        angles.x = -75.f;
    else if (angles.x > 75.f)
        angles.x = 75.f;
    while (angles.y < -180.0f)
        angles.y += 360.0f;
    while (angles.y > 180.0f)
        angles.y -= 360.0f;
}

// Legacy top-left FPS counter removed; menu header now owns the FPS display.


ImColor outlinecolor = IM_COL32(10, 10, 10, 255);

void drawESP__Player_Legacy(ImDrawList *draw) {   
        if (!started)
        return;
        
    int totalEnemies = 0;
    int totalBots = 0;
    float kmdis = 0;
    char extra[30];
    
    
    
                      
      
   
    
    if (dropdownStates["#DIST"] == EAim::Crosshair) {
        draw->AddCircle(ImVec2(glWidth / 2, glHeight / 2), seekbarValues["FOV500"]*2.0f, ImColor(255,0,0,255), 0, 0.0f);
    }
    
    
    
      
    if (checkboxStates["ESP ENABLE"]) {
         
        Vector3 pLocalPawn_rootPos = Vector3::zero();     
        Pawn *get_LocalPawn = GamePlay::get_LocalPawn();
        if (Tools::IsPtrValid(get_LocalPawn)) {
        pLocalPawn_rootPos = get_LocalPawn->get_LastPawnPos();         
        }

    
    int totalEnemies = 0;
    int totalBots = 0;
    char extra[30];
    
  

        BaseGame *get_MatchGame = GamePlay::get_MatchGame();
        if (Tools::IsPtrValid(get_MatchGame)) {
           
            List<Pawn *> *EnemyPawns = get_MatchGame->EnemyPawns();
            Camera *mainCamera = Camera::get_main();
            if (Tools::IsPtrValid(EnemyPawns) && EnemyPawns->getSize() &&
                Tools::IsPtrValid(mainCamera)) {
                Pawn **pawns = (Pawn **) EnemyPawns->getItems();
                for (int i = 0; pawns != nullptr && i < EnemyPawns->getSize(); i++) {
                        
                    Pawn *pawn = pawns[i];
                    if (Tools::IsPtrValid(pawn)) {     
                    
                                      
                        auto m_HeadBone = *(Transform **) (pawn + Class_Pawn_m_HeadBone);
                        if (!m_HeadBone)
                        continue;                   
                       
                        auto m_Mesh = *(Transform **) (pawn + Class_Pawn_m_Mesh);
                        if (!m_Mesh)
                        continue;
                        
                        auto m_alive = *(bool*) (pawn + Class_Pawn_m_IsAlive);
                        if (!m_alive)
                        continue;               
                    
                    
                        
                        Vector3 pEnemyPawn_headPos = pawn->get_HeadPosition();
                        Vector3 pEnemyPawn_rootPos = pawn->get_LastPawnPos();
                                                 
                        int is_NameColors;
                        int is_Color;
                        int is_Color2;
                        long is_ColorFilled;                        
                        long Alert_Color;                     
                                                
                        
                        if (pawn->m_IsBot()) {
                     
                            is_Color = ImColor(0, 255, 0, 255);
                            is_ColorFilled = ImColor(3, 255, 40, 60);
                            is_NameColors = ImColor(0, 255, 0, 255);
                                                                                 
                            totalBots++;
                            Alert_Color = IM_COL32(0, 255, 0, 255);
                        } else {
                            is_Color2 = ImColor(255, 0, 0, 255);
                            is_ColorFilled = ImColor(255, 0, 0, 60);
                            is_NameColors = ImColor(255, 255, 100, 255);
                                                  
                            totalEnemies++;
                            Alert_Color = IM_COL32(255, 0, 0, 255);
                        }
                        
                        
                        
                        Vector3 headPos_WS = mainCamera->WorldToScreenPoint(pEnemyPawn_headPos);
                        Vector3 rootPos_WS = mainCamera->WorldToScreenPoint(pEnemyPawn_rootPos);
         
                        float distanceToMe = Vector3::Distance(pLocalPawn_rootPos, pEnemyPawn_rootPos);
                        if (distanceToMe <= 500.0f) {  
                            
                            if (pawn->m_IsAlive()) {
                      
                                Vector2 screen(glWidth, glHeight);
                                Vector2 location(rootPos_WS.x,headPos_WS.y);
                                float magic_number = (distanceToMe);
                                float mx = (glWidth / 6) / magic_number;
                                float healthLength = glWidth / 20;
                                    if (healthLength < mx)
                                        healthLength = mx;
                                
                                if (headPos_WS.z > 0 && rootPos_WS.z > 0) {
                            
                                    //ESP Line
                                    if (checkboxStates["ESP LINE"]) {
                                        if (pawn->m_IsBot()) {
                                        
                                        drawLine(is_Color, 1.7f, Vector3(glWidth / 2, 80), Vector3(headPos_WS.x, glHeight - headPos_WS.y));
                                        } else {
                                        drawLine(is_Color2, 1.7f, Vector3(glWidth / 2, 80), Vector3(headPos_WS.x, glHeight - headPos_WS.y));
                                    }
                                    }
                                    //ESP Box
                                    if (checkboxStates["ESP BOX"]) {
                                        if (dropdownStates["#BOX"] == 0) {
                                        float boxHeight = abs(headPos_WS.y - rootPos_WS.y);
                                        float boxWidth = boxHeight * 0.65f;
                                        
                                        ImVec2 vStart = {rootPos_WS.x - (boxWidth / 2), glHeight - rootPos_WS.y - boxHeight};
                                        ImVec2 vEnd = {vStart.x + boxWidth, vStart.y + boxHeight};
                                        
                                        draw->AddRectFilled(vStart, vEnd, is_ColorFilled, 0.5f);
                                        draw->AddRect(vStart, vEnd, ImColor(255, 255, 255, 255), 1.0f);
                                        }
                                        
                                        if (dropdownStates["#BOX"] == 1) {
                                            float boxHeight = abs(headPos_WS.y - rootPos_WS.y);
                                        float boxWidth = boxHeight * 0.65f;
                                      //  int x = rootPos_WS.x - (boxWidth / 2);
                                        int x = rootPos_WS.x - (boxWidth / 2);
                                        int y = glHeight - headPos_WS.y;
                                        int w = boxWidth;
                                        int h = boxHeight;
                                        int iw = w / 4;
                                        int ih = h / 4;                                                             
                                        //Top
                                        draw->AddLine(ImVec2(x, y), ImVec2(x + iw, y), ImColor(255, 255, 255, 255), 1.8f);
                                        draw->AddLine(ImVec2(x + w - iw, y), ImVec2(x + w, y), ImColor(255, 255, 255, 255), 1.8f);
                                        draw->AddLine(ImVec2(x, y), ImVec2(x, y + ih), ImColor(255, 255, 255, 255), 1.8f);
                                        draw->AddLine(ImVec2(x + w - 1, y), ImVec2(x + w - 1, y + ih), ImColor(255, 255, 255, 255), 1.8f);
                                        // bottom
                                        draw->AddLine(ImVec2(x, y + h), ImVec2(x + iw, y + h), ImColor(255, 255, 255, 255), 1.8f);
                                        draw->AddLine(ImVec2(x + w - iw, y + h), ImVec2(x + w, y + h), ImColor(255, 255, 255, 255), 1.8f);
                                        draw->AddLine(ImVec2(x, y + h - ih), ImVec2(x, y + h), ImColor(255, 255, 255, 255), 1.8f);
                                        draw->AddLine(ImVec2(x + w - 1, y + h - ih), ImVec2(x + w - 1, y + h), ImColor(255, 255, 255, 255), 1.8f);
                                        //draw->AddRect(vStart, vEnd, ImColor(255, 255, 255, 255), 1.0f);
                                            }

                                        if (dropdownStates["#BOX"] == 2) {
                                            float boxHeight = abs(headPos_WS.y - rootPos_WS.y);
                                            float boxWidth = boxHeight * 0.65f;
                                            ImVec2 vStart = {rootPos_WS.x - (boxWidth / 2), glHeight - rootPos_WS.y - boxHeight};
                                            ImVec2 vEnd = {vStart.x + boxWidth, vStart.y + boxHeight};
                                            draw->AddRect(vStart, vEnd, ImColor(255,255,255,255), 0.5f, 0, 1.5f);
                                        }
                                    }
                                    
                                    if (checkboxStates["ESPREDDOT"]) {
    // Draw a red dot instead of a box
    ImVec2 redDotPos = ImVec2(headPos_WS.x, glHeight - headPos_WS.y); // Screen Y is usually inverted
    float dotSize = 4.0f; // Size of the dot (adjust if needed)
    
    draw->AddCircleFilled(redDotPos, dotSize, ImColor(255, 0, 0, 255), 12); // 12 segments for smooth circle
}

                                    
                                    //Box Text
                                    if (checkboxStates["ESP NAME"] || checkboxStates["ESP DISTANCE"] || checkboxStates["ESP PLAYER ID"] || checkboxStates["ESP HEALTH"] || checkboxStates["ESP ALERT 360"]) {
                                        
                                      DrawBoxEnemy(ImVec2(headPos_WS.x - glWidth / 20, glHeight - headPos_WS.y - 16.0f), ImVec2(headPos_WS.x + glWidth / 20, glHeight - headPos_WS.y - 16.0f), 22, IM_COL32(0, 0, 0, 190));
                                        
                                       /* int get_TeamSeatId = 0;
                                        
                                        draw->AddRectFilled({headPos_WS.x - 82, glHeight - headPos_WS.y - 27}, {headPos_WS.x + 100, glHeight - headPos_WS.y - 8}, ImColor(0, 0, 0, 255), 1, 0); // DrawBoxEnemy
                                        //draw->AddRectFilled({headPos_WS.x - 82, glHeight - headPos_WS.y - 31}, {headPos_WS.x - 50, glHeight - headPos_WS.y - 9}, GetRandomColorByIndex(get_TeamSeatId), 1, 0); // TeamID*/
                                        
                                    }
                                    
                                    
                                    //ESP Name
                                    if (checkboxStates["ESP NAME"]) {
                                        //std::string strName;                                      
                                        std::string strName = pawn->get_PlayerName()->CString();
                                        auto textSize = ImGui::CalcTextSize(strName.c_str(), 0);     
                                        //std::string strName = pawn->get_PlayerName()->CString();
                                        draw->AddText(nullptr, 15, ImVec2(headPos_WS.x - 35.0f, glHeight - headPos_WS.y - 23.0f), ImColor(255, 255, 255, 255), strName.c_str());
                                        /*draw->AddText(nullptr, 18, ImVec2(headPos_WS.x - 45.0f, glHeight - headPos_WS.y - 27.0f), ImColor(0, 0, 0, 255), strName.c_str());
                                        draw->AddText(nullptr, 18, ImVec2(headPos_WS.x - 46.0f, glHeight - headPos_WS.y - 28.0f), ImColor(255, 255, 255, 255), strName.c_str());*/
                                    }
                                    
                                    //ESP PlayerID
                                    if (checkboxStates["ESP PLAYER ID"]) {
                                       /* std::string strID;
                                        auto textSize = ImGui::CalcTextSize(strID.c_str(), 0);                                    
                                        strID +=" ";
                                        strID += std::to_string((int) pawn->get_TeamSeatId());                                                                        
                                        //DrawBoxEnemy(ImVec2(headPos_WS.x - 50.0f, glHeight - headPos_WS.y - 20.0f), ImVec2(headPos_WS.x - 80.0f, glHeight - headPos_WS.y - 20.0f), 22, IM_COL32(255, 255, 255, 255));
                                        
                                        draw->AddText(nullptr, 25, ImVec2(headPos_WS.x - 75.0f - (textSize.x / 2), glHeight - headPos_WS.y - 30.0f), ImColor(0, 0, 0, 255), strID.c_str());
                                        draw->AddText(nullptr, 25, ImVec2(headPos_WS.x - 76.0f - (textSize.x / 2), glHeight - headPos_WS.y - 31.0f), ImColor(255, 255, 255, 255), strID.c_str());
                                        //draw->AddText(nullptr, 17, ImVec2(headPos_WS.x - 65.0f - (textSize.x / 2), glHeight - headPos_WS.y - 25.0f), ImColor(0, 0, 0, 255), strID.c_str());*/
                                        
                                    
                  std::string strID = to_string((int) pawn->get_TeamSeatId());  
                  int TeamID = std::stoi(strID);
                    ImU32 tColors;
                    
                    if (TeamID < 100) {
                        tColors = teamIdColors[0];
                    }
                    if (TeamID < 96) {
                        tColors = teamIdColors[1];
                    }
                    if (TeamID < 92) {
                        tColors = teamIdColors[2];
                    }
                    if (TeamID < 88) {
                        tColors = teamIdColors[3];
                    }
                    if (TeamID < 84) {
                        tColors = teamIdColors[4];
                    }
                    if (TeamID < 80) {
                        tColors = teamIdColors[5];
                    }
                    if (TeamID < 76) {
                        tColors = teamIdColors[6];
                    }
                    if (TeamID < 72) {
                        tColors = teamIdColors[7];
                    }
                    if (TeamID < 68) {
                        tColors = teamIdColors[8];
                    }
                    if (TeamID < 64) {
                        tColors = teamIdColors[9];
                    }
                    if (TeamID < 60) {
                        tColors = teamIdColors[10];
                    }
                    if (TeamID < 56) {
                        tColors = teamIdColors[11];
                    }
                    if (TeamID < 52) {
                        tColors = teamIdColors[12];
                    }
                    if (TeamID < 48) {
                        tColors = teamIdColors[13];
                    }
                    if (TeamID < 44) {
                        tColors = teamIdColors[14];
                    }
                    if (TeamID < 40) {
                        tColors = teamIdColors[15];
                    }
                    if (TeamID < 36) {
                        tColors = teamIdColors[16];
                    }
                    if (TeamID < 32) {
                        tColors = teamIdColors[17];
                    }
                    if (TeamID < 28) {
                        tColors = teamIdColors[18];
                    }
                    if (TeamID < 24) {
                        tColors = teamIdColors[19];
                    }
                    if (TeamID < 20) {
                        tColors = teamIdColors[20];
                    }
                    if (TeamID < 16) {
                        tColors = teamIdColors[21];
                    }
                    if (TeamID < 12) {
                        tColors = teamIdColors[22];
                    }
                    if (TeamID < 8) {
                        tColors = teamIdColors[23];
                    }
                    if (TeamID < 4) {
                        tColors = teamIdColors[24];
                    }
                                        
                                        
                                        DrawBoxEnemy(ImVec2(headPos_WS.x - 40.0f, glHeight - headPos_WS.y - 16.0f), ImVec2(headPos_WS.x - 80.0f, glHeight - headPos_WS.y - 16.0f), 22, tColors);
                                        //draw->AddText(nullptr, 15, ImVec2(headPos_WS.x - 64.0f, glHeight - headPos_WS.y - 24.0f), ImColor(255, 255, 255, 255));
                                    }
                                    
                                    
                                    
                                    /*
                                    //ESP InfoWeapon
                                    if (Config.ExtraMenu.PlayerWeapon) {
                                        std::string strID = EWeaponNameToString(pawn->m_LastFireWeaponName());
                                        draw->AddText(nullptr, 20, ImVec2(headPos_WS.x - 77.0f, glHeight - headPos_WS.y - 46.0f), ImColor(0, 0, 0, 255), strID.c_str());
                                    }
                                    */
                                    
                                    
                                    // ESP Foot - animated 3D floor glow under the player's feet.
                                    if (checkboxStates["ESP FOOT"]) {
                                        const float now = (float)ImGui::GetTime();
                                        const float boxHeight = std::max(42.0f, (float)abs(headPos_WS.y - rootPos_WS.y));
                                        const float rxBase = std::max(18.0f, std::min(52.0f, boxHeight * 0.24f));
                                        const float ryBase = std::max(6.0f, std::min(17.0f, rxBase * 0.30f));
                                        const float pulse = 0.5f + 0.5f * sinf(now * 4.4f + (float)i * 0.47f);
                                        const ImVec2 footCenter(rootPos_WS.x, glHeight - rootPos_WS.y + 7.0f);
                                        const int pointCount = 48;
                                        ImVec2 points[pointCount];

                                        // translucent floor pool for a soft 3D base
                                        for (int p = 0; p < pointCount; ++p) {
                                            const float a = (6.28318530718f * (float)p) / (float)pointCount;
                                            points[p] = ImVec2(footCenter.x + cosf(a) * (rxBase - 3.0f),
                                                               footCenter.y + sinf(a) * (ryBase - 1.0f));
                                        }
                                        draw->AddConvexPolyFilled(points, pointCount,
                                                                  IM_COL32(78, 213, 255, (int)(22.0f + 18.0f*pulse)));

                                        // layered neon rings create the glowing/3D illusion
                                        for (int layer = 3; layer >= 0; --layer) {
                                            const float expand = (float)layer * 2.6f + pulse * 1.4f;
                                            const float rx = rxBase + expand;
                                            const float ry = ryBase + expand * 0.34f;
                                            for (int p = 0; p < pointCount; ++p) {
                                                const float a = (6.28318530718f * (float)p) / (float)pointCount;
                                                points[p] = ImVec2(footCenter.x + cosf(a) * rx,
                                                                   footCenter.y + sinf(a) * ry);
                                            }
                                            const int alpha = 48 + (3-layer) * 38;
                                            const ImU32 ringColor = (layer & 1)
                                                ? IM_COL32(145, 95, 255, alpha)
                                                : IM_COL32(70, 225, 255, alpha);
                                            draw->AddPolyline(points, pointCount, ringColor, ImDrawFlags_Closed,
                                                              layer == 0 ? 2.3f : 1.5f);
                                        }

                                        // moving sparkles orbit the ring and blink independently
                                        for (int sp = 0; sp < 5; ++sp) {
                                            const float a = now * (1.1f + 0.08f*sp) + (6.28318530718f * sp / 5.0f);
                                            const float blink = 0.5f + 0.5f * sinf(now * 6.0f + sp * 1.7f);
                                            const ImVec2 q(footCenter.x + cosf(a) * (rxBase + 4.0f),
                                                           footCenter.y + sinf(a) * (ryBase + 2.0f));
                                            const float sparkleR = 1.4f + 1.6f * blink;
                                            draw->AddCircleFilled(q, sparkleR + 3.0f, IM_COL32(91,220,255,(int)(24+34*blink)), 12);
                                            draw->AddCircleFilled(q, sparkleR, IM_COL32(238,252,255,(int)(150+105*blink)), 12);
                                            draw->AddLine(ImVec2(q.x-4.0f*blink,q.y), ImVec2(q.x+4.0f*blink,q.y),
                                                          IM_COL32(238,252,255,(int)(80+120*blink)), 1.0f);
                                            draw->AddLine(ImVec2(q.x,q.y-3.0f*blink), ImVec2(q.x,q.y+3.0f*blink),
                                                          IM_COL32(238,252,255,(int)(80+120*blink)), 1.0f);
                                        }
                                    }

                                    // ESP Health - glossy game-style bar with green -> yellow -> orange -> red depletion.
                                    if (checkboxStates["ESP HEALTH"]) {
                                        int get_Health = 0;
                                        int max_Health = 0;
                                        AttackableTargetInfo *m_AttackableInfo = pawn->m_AttackableInfo();
                                        if (m_AttackableInfo != nullptr) {
                                            get_Health = m_AttackableInfo->m_Health();
                                            max_Health = m_AttackableInfo->m_MaxHealth();
                                        }

                                        if (max_Health > 0) {
                                            const float hpRatio = std::max(0.0f, std::min(1.0f, (float)get_Health / (float)max_Health));
                                            int hpR = 255;
                                            int hpG = 0;
                                            if (hpRatio >= 0.5f) {
                                                hpR = (int)(255.0f * (1.0f - hpRatio) * 2.0f);
                                                hpG = 255;
                                            } else {
                                                hpR = 255;
                                                hpG = (int)(255.0f * hpRatio * 2.0f);
                                            }
                                            const ImU32 hpColor = IM_COL32(hpR, hpG, 18, 255);

                                            if (dropdownStates["#HEALTH"] == 0) {
                                                const float fullW = std::max(56.0f, healthLength * 2.0f);
                                                const float barH = 10.0f;
                                                const ImVec2 barMin(headPos_WS.x - fullW * 0.5f, glHeight - headPos_WS.y - 38.0f);
                                                const ImVec2 barMax(barMin.x + fullW, barMin.y + barH);
                                                const ImVec2 fillMax(barMin.x + fullW * hpRatio, barMax.y);

                                                // compact red heart marker, inspired by the supplied game-style reference
                                                const ImVec2 heartC(barMin.x - 9.0f, barMin.y + barH * 0.50f);
                                                draw->AddCircleFilled(ImVec2(heartC.x-3.0f, heartC.y-2.0f), 4.2f, IM_COL32(255,42,42,255), 14);
                                                draw->AddCircleFilled(ImVec2(heartC.x+3.0f, heartC.y-2.0f), 4.2f, IM_COL32(255,42,42,255), 14);
                                                draw->AddTriangleFilled(ImVec2(heartC.x-7.0f, heartC.y-1.0f),
                                                                        ImVec2(heartC.x+7.0f, heartC.y-1.0f),
                                                                        ImVec2(heartC.x, heartC.y+8.0f),
                                                                        IM_COL32(255,42,42,255));
                                                draw->AddCircleFilled(ImVec2(heartC.x-2.4f, heartC.y-3.0f), 1.3f, IM_COL32(255,220,220,230), 8);

                                                // soft depth/glow shadow
                                                draw->AddRectFilled(ImVec2(barMin.x-3.0f, barMin.y-3.0f),
                                                                    ImVec2(barMax.x+3.0f, barMax.y+3.0f),
                                                                    IM_COL32(hpR, hpG, 18, 42), 5.0f);
                                                draw->AddRectFilled(barMin, barMax, IM_COL32(12, 14, 15, 230), 3.0f);
                                                if (fillMax.x > barMin.x + 1.0f) {
                                                    draw->AddRectFilled(barMin, fillMax, hpColor, 3.0f);
                                                    // glossy highlight like the supplied pixel health-bar reference
                                                    draw->AddRectFilled(ImVec2(barMin.x+2.0f, barMin.y+1.5f),
                                                                        ImVec2(std::max(barMin.x+2.0f, fillMax.x-2.0f), barMin.y+4.2f),
                                                                        IM_COL32(255,255,255,58), 2.0f);
                                                    draw->AddRectFilled(ImVec2(barMin.x+2.0f, barMax.y-3.0f),
                                                                        ImVec2(std::max(barMin.x+2.0f, fillMax.x-2.0f), barMax.y-1.0f),
                                                                        IM_COL32(0,0,0,42), 1.0f);
                                                }
                                                draw->AddRect(barMin, barMax, IM_COL32(0,0,0,255), 3.0f, ImDrawFlags_RoundCornersAll, 2.0f);

                                                // subtle 25% separators keep the bar readable without making it too busy
                                                for (int seg = 1; seg < 4; ++seg) {
                                                    const float sx = barMin.x + fullW * ((float)seg / 4.0f);
                                                    draw->AddLine(ImVec2(sx, barMin.y+2.0f), ImVec2(sx, barMax.y-2.0f),
                                                                  IM_COL32(0,0,0,78), 1.0f);
                                                }
                                            }

                                            if (dropdownStates["#HEALTH"] == 1) {
                                                const float boxHeight = std::max(24.0f, (float)abs(headPos_WS.y - rootPos_WS.y));
                                                const float boxWidth = boxHeight * 0.65f;
                                                const float fullH = boxHeight;
                                                const float barW = 10.0f;
                                                const ImVec2 vA(headPos_WS.x + (boxWidth / 2.0f) + 8.0f, glHeight - headPos_WS.y);
                                                const ImVec2 vB(vA.x + barW, vA.y + fullH);
                                                const ImVec2 fillA(vA.x, vB.y - fullH * hpRatio);

                                                draw->AddRectFilled(vA, vB, IM_COL32(12,14,15,230), 3.0f);
                                                draw->AddRectFilled(fillA, vB, hpColor, 3.0f);
                                                draw->AddRect(ImVec2(vA.x-1.0f,vA.y-1.0f), ImVec2(vB.x+1.0f,vB.y+1.0f),
                                                              IM_COL32(0,0,0,255), 3.0f, ImDrawFlags_RoundCornersAll, 2.0f);
                                            }
                                        }
                                    }
                                        
                                        
                                       /* int get_Health = 0;
                                        int max_Health = 0;
                                        
                                        AttackableTargetInfo *m_AttackableInfo = pawn->m_AttackableInfo();
                                        if (m_AttackableInfo != nullptr) {
                                            get_Health = m_AttackableInfo->m_Health();
                                            max_Health = m_AttackableInfo->m_MaxHealth();
                                        }
                                        
                                        drawHorizontalHealthBar(Vector3(headPos_WS.x - 28, glHeight - headPos_WS.y), 180.0f, max_Health, get_Health);
                                    }*/
                                    
                                    //ESP Distance
                                    if (checkboxStates["ESP DISTANCE"]) {
                                        std::string strDistance;
                                        auto textSize = ImGui::CalcTextSize(strDistance.c_str(), 0);
                                        strDistance += std::to_string((int) distanceToMe) + "m";
                                       //std::string strDistance = to_string((int) distanceToMe) + "  M";
                                        draw->AddText(nullptr, 15.0f, ImVec2(headPos_WS.x + 47.0f, glHeight - headPos_WS.y - 23.0f), ImColor(255, 255, 255, 255), strDistance.c_str());
                                    } 
                                        
                                       /* draw->AddText(nullptr, 18, ImVec2(headPos_WS.x + 70.0f, glHeight - headPos_WS.y - 27.0f), ImColor(0, 0, 0, 255), strDistance.c_str());
                                        draw->AddText(nullptr, 18, ImVec2(headPos_WS.x + 71.0f, glHeight - headPos_WS.y - 28.0f), ImColor(255, 255, 255, 255), strDistance.c_str()); */
                                    
                                    
                                    if (checkboxStates["ESP ALERT 360"]) {
                                        
                                        Vector3 angle = Vector3();
                                        Vector3 forward = Vector3((float)(glWidth / 2) - headPos_WS.x, (float)(glHeight / 2) - headPos_WS.y, 0.0f);  
                                        VectorAnglesRadar(forward, angle);
                                        const auto angle_yaw_rad = DEG2RAD(angle.y + 180.f);
                                        const auto new_point_x = (glWidth / 2) + (55/*alert dist from me*/) / 2 * 8 * cosf(angle_yaw_rad);  
                                        const auto new_point_y = (glHeight / 2) + (55/*alert dist from me*/) / 2 * 8 * sinf(angle_yaw_rad);  
                                        std::array <Vector3, 3 > points {Vector3(new_point_x - ((90) / 4 + 3.5f) / 2, new_point_y - ((55) / 4 + 3.5f) / 2, 0.f), Vector3(new_point_x + ((90) / 4 + 3.5f) / 4, new_point_y, 0.f), Vector3(new_point_x - ((90) / 4 + 3.5f) / 2, new_point_y + ((55) / 4 + 3.5f) / 2, 0.f)};
                                        std::string strDistance;                                        
                                        auto textSize = ImGui::CalcTextSize(strDistance.c_str(), 0, ((float) density / 20.0f));
                                        strDistance += std::to_string((int) distanceToMe) + "m";
                                        draw->AddText(NULL, ((float) density / 20.0f), {new_point_x - (textSize.x / 2), new_point_y + 7.f}, IM_COL32(255,255,255,255), strDistance.c_str());                 
                                        RotateTriangle(points, angle.y + 180.f);  
                                    if (pawn->m_IsBot()) {
                                        draw->AddTriangle(ImVec2(points.at(0).x, points.at(0).y), ImVec2(points.at(1).x, points.at(1).y), ImVec2(points.at(2).x, points.at(2).y), IM_COL32(0, 255, 0, 255), 1.5f);
                                        draw->AddTriangleFilled(ImVec2(points.at(0).x, points.at(0).y), ImVec2(points.at(1).x, points.at(1).y), ImVec2(points.at(2).x, points.at(2).y), IM_COL32(0, 255, 0, 255));
                                        } else {
                                        draw->AddTriangle(ImVec2(points.at(0).x, points.at(0).y), ImVec2(points.at(1).x, points.at(1).y), ImVec2(points.at(2).x, points.at(2).y), IM_COL32(255, 0, 0, 255), 1.5f);
                                        draw->AddTriangleFilled(ImVec2(points.at(0).x, points.at(0).y), ImVec2(points.at(1).x, points.at(1).y), ImVec2(points.at(2).x, points.at(2).y), IM_COL32(255, 0, 0, 255));                            
                                        }
                                    //}
                                    //}
                                    //}
                                    
                                    
                                    
                                    
                                }
                        }
                    }
                }
            }
        }
    }
}

        
           if (checkboxStates["ESP COUNT"]) {
                  
          int TotalCount = totalEnemies;
    
    ImGui::GetForegroundDrawList()->AddRectFilled({glWidth /2-104.953,40},{glWidth /2- -86.085,80},ImColor(8, 8, 7,180), 50.0f);
  //  ImGui::GetForegroundDrawList()->AddRectFilled({glWidth /2+80,40},{glWidth /2+20,80},ImColor(72,255,0,255));     
    sprintf(extra, "Players: %d  ", totalEnemies);
    ImGui::GetForegroundDrawList()->AddText({glWidth /2-97.877,45}, ImColor(255, 255, 252), extra);
    sprintf(extra, "Bots: %d ", totalBots);
    ImGui::GetForegroundDrawList()->AddText({glWidth /2+15.330,45}, ImColor(255, 255, 252), extra);
}
       }   
    }
    
// Cyrus V4-style renderer adapted to this source's checkboxStates and glWidth/glHeight.
// The old renderer is retained above but is not called, avoiding its unchecked name/health reads.
static void drawESP__Player_CyrusStyle(ImDrawList* draw) {
    if (!draw || !started || !checkboxStates["ESP ENABLE"] || glWidth <= 0 || glHeight <= 0)
        return;

    // Do not enter IL2CPP accessors until the needed method addresses are initialized.
    if (!Class_Camera_get_main || !Class_Camera_WorldToScreenPoint ||
        !Class_Gameplay_get_MatchGame || !Class_Gameplay_get_LocalPawn ||
        !Class_Transform_get_position || !Class_Pawn_get_LastPawnPos ||
        !Class_BaseGame_EnemyPawns || !Class_Pawn_m_HeadBone || !Class_Pawn_m_Mesh ||
        !Class_Pawn_m_IsAlive || !Class_Pawn_m_IsBot)
        return;

    if (!StretchESPIsReadable(reinterpret_cast<void*>(Class_Camera_get_main), sizeof(void*)) ||
        !StretchESPIsReadable(reinterpret_cast<void*>(Class_Camera_WorldToScreenPoint), sizeof(void*)) ||
        !StretchESPIsReadable(reinterpret_cast<void*>(Class_Transform_get_position), sizeof(void*)) ||
        !StretchESPIsReadable(reinterpret_cast<void*>(Class_Pawn_get_LastPawnPos), sizeof(void*))) return;
    Camera* camera = Camera::get_main();
    if (!StretchESPIsReadable(camera, sizeof(void*))) return;

    auto getMatchFn = reinterpret_cast<MatchGame*(*)()>(Class_Gameplay_get_MatchGame);
    auto getLocalFn = reinterpret_cast<Pawn*(*)()>(Class_Gameplay_get_LocalPawn);
    if (!StretchESPIsReadable(reinterpret_cast<void*>(getMatchFn), sizeof(void*)) ||
        !StretchESPIsReadable(reinterpret_cast<void*>(getLocalFn), sizeof(void*))) return;

    MatchGame* match = getMatchFn();
    Pawn* localPawn = getLocalFn();
    if (!StretchESPIsReadable(match, sizeof(void*)) || !StretchESPIsReadable(localPawn, sizeof(void*))) return;

    // These methods dereference fields internally, so validate each field address first.
    if (!StretchESPIsReadable(reinterpret_cast<void*>((uintptr_t)match + Class_BaseGame_EnemyPawns), sizeof(void*))) return;
    List<Pawn*>* enemyList = match->EnemyPawns();
    if (!StretchESPIsReadable(enemyList, sizeof(void*) * 5)) return;

    int enemyCount = 0;
    if (!StretchESPRead((uintptr_t)enemyList + sizeof(void*) * 3, enemyCount)) return;
    if (enemyCount <= 0 || enemyCount > 128) return;

    // The List layout is klass, sync/root, items, size, version.
    Il2CppArray<Pawn*>* itemArray = nullptr;
    if (!StretchESPRead((uintptr_t)enemyList + sizeof(void*) * 2, itemArray) ||
        !StretchESPIsReadable(itemArray, sizeof(void*) * 4)) return;
    Pawn** pawns = itemArray->getPointer();
    if (!StretchESPIsReadable(pawns, sizeof(Pawn*) * (size_t)enemyCount)) return;

    auto getPawnPosition = reinterpret_cast<Vector3(*)(Pawn*)>(Class_Pawn_get_LastPawnPos);
    auto getTransformPosition = reinterpret_cast<Vector3(*)(Transform*)>(Class_Transform_get_position);
    auto worldToScreen = reinterpret_cast<Vector3(*)(Camera*, Vector3)>(Class_Camera_WorldToScreenPoint);
    Vector3 localWorld = getPawnPosition(localPawn);
    if (!StretchESPFinite3(localWorld)) return;

    const float scale = std::max(0.75f, std::min(1.7f, (float)glHeight / 720.0f));
    const float fontSize = std::max(12.0f, 14.0f * scale);
    int totalBots = 0;
    int totalPlayers = 0;
    const float now = (float)ImGui::GetTime();

    for (int i = 0; i < enemyCount; ++i) {
        Pawn* pawn = nullptr;
        if (!StretchESPRead((uintptr_t)pawns + sizeof(Pawn*) * (size_t)i, pawn) ||
            !StretchESPIsReadable(pawn, sizeof(void*))) continue;

        bool alive = false, isBot = false;
        if (!StretchESPRead((uintptr_t)pawn + Class_Pawn_m_IsAlive, alive) || !alive) continue;
        if (!StretchESPRead((uintptr_t)pawn + Class_Pawn_m_IsBot, isBot)) continue;
        if (isBot) ++totalBots; else ++totalPlayers;

        Transform* headBone = nullptr;
        Transform* mesh = nullptr;
        if (!StretchESPRead((uintptr_t)pawn + Class_Pawn_m_HeadBone, headBone) ||
            !StretchESPRead((uintptr_t)pawn + Class_Pawn_m_Mesh, mesh) ||
            !StretchESPIsReadable(headBone, sizeof(void*)) || !StretchESPIsReadable(mesh, sizeof(void*))) continue;
        if (!StretchESPIsReadable(reinterpret_cast<void*>(Class_Transform_get_position), sizeof(void*))) continue;

        const Vector3 headWorld = getTransformPosition(headBone);
        const Vector3 rootWorld = getTransformPosition(mesh);
        if (!StretchESPFinite3(headWorld) || !StretchESPFinite3(rootWorld)) continue;

        const float dx = localWorld.x - rootWorld.x;
        const float dy = localWorld.y - rootWorld.y;
        const float dz = localWorld.z - rootWorld.z;
        const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (!std::isfinite(distance) || distance < 0.1f || distance > 500.0f) continue;

        const Vector3 headScreen = worldToScreen(camera, headWorld);
        const Vector3 rootScreen = worldToScreen(camera, rootWorld);
        if (!StretchESPFinite3(headScreen) || !StretchESPFinite3(rootScreen) ||
            headScreen.z <= 0.0f || rootScreen.z <= 0.0f) continue;

        const float headX = headScreen.x;
        const float headY = (float)glHeight - headScreen.y;
        const float rootY = (float)glHeight - rootScreen.y;
        float boxHeight = std::fabs(rootY - headY);
        if (!std::isfinite(boxHeight) || boxHeight < 4.0f || boxHeight > (float)glHeight * 1.5f) continue;
        const float boxWidth = std::max(8.0f, boxHeight * 0.48f);
        const ImVec2 topLeft(headX - boxWidth * 0.5f, std::min(headY, rootY));
        const ImVec2 bottomRight(headX + boxWidth * 0.5f, std::max(headY, rootY));
        const ImU32 playerColor = isBot ? IM_COL32(35, 235, 95, 245) : IM_COL32(255, 65, 65, 245);
        const ImU32 fillColor = isBot ? IM_COL32(35, 235, 95, 34) : IM_COL32(255, 65, 65, 34);

        // ESP lines — source toggle preserved.
        if (checkboxStates["ESP LINE"]) {
            draw->AddLine(ImVec2(glWidth * 0.5f, 80.0f * scale), ImVec2(headX, headY), playerColor, 1.5f * scale);
        }

        // ESP boxes — same three style options, with Cyrus-style team coloring.
        if (checkboxStates["ESP BOX"]) {
            const int style = dropdownStates["#BOX"];
            if (style == 0) {
                draw->AddRectFilled(topLeft, bottomRight, fillColor, 3.0f * scale);
                draw->AddRect(topLeft, bottomRight, playerColor, 3.0f * scale, 0, 1.5f * scale);
            } else if (style == 1) {
                const float w = bottomRight.x - topLeft.x;
                const float h = bottomRight.y - topLeft.y;
                const float cw = w * 0.27f, ch = h * 0.22f;
                const ImVec2 tl = topLeft, br = bottomRight;
                draw->AddLine(tl, ImVec2(tl.x + cw, tl.y), playerColor, 2.0f * scale);
                draw->AddLine(tl, ImVec2(tl.x, tl.y + ch), playerColor, 2.0f * scale);
                draw->AddLine(ImVec2(br.x - cw, tl.y), ImVec2(br.x, tl.y), playerColor, 2.0f * scale);
                draw->AddLine(ImVec2(br.x, tl.y), ImVec2(br.x, tl.y + ch), playerColor, 2.0f * scale);
                draw->AddLine(ImVec2(tl.x, br.y - ch), ImVec2(tl.x, br.y), playerColor, 2.0f * scale);
                draw->AddLine(ImVec2(tl.x, br.y), ImVec2(tl.x + cw, br.y), playerColor, 2.0f * scale);
                draw->AddLine(ImVec2(br.x - cw, br.y), br, playerColor, 2.0f * scale);
                draw->AddLine(ImVec2(br.x, br.y - ch), br, playerColor, 2.0f * scale);
            } else {
                draw->AddRect(topLeft, bottomRight, playerColor, 0.0f, 0, 1.8f * scale);
            }
        }

        // ESP Foot Glow — retained; skip drawing if foot coordinates are not finite.
        if (checkboxStates["ESP FOOT"]) {
            const ImVec2 foot(rootScreen.x, rootY + 5.0f * scale);
            if (std::isfinite(foot.x) && std::isfinite(foot.y)) {
                const float rx = std::max(14.0f, std::min(48.0f, boxHeight * 0.24f));
                const float ry = std::max(5.0f, std::min(15.0f, rx * 0.30f));
                const float pulse = 0.5f + 0.5f * sinf(now * 4.4f + (float)i * 0.47f);
                const int pts = 40;
                ImVec2 ring[pts];
                for (int p = 0; p < pts; ++p) {
                    const float a = 6.28318530718f * (float)p / (float)pts;
                    ring[p] = ImVec2(foot.x + cosf(a) * (rx - 2.0f), foot.y + sinf(a) * (ry - 1.0f));
                }
                draw->AddConvexPolyFilled(ring, pts, IM_COL32(78, 213, 255, (int)(20.0f + 16.0f * pulse)));
                for (int layer = 3; layer >= 0; --layer) {
                    const float expand = layer * 2.2f + pulse * 1.4f;
                    for (int p = 0; p < pts; ++p) {
                        const float a = 6.28318530718f * (float)p / (float)pts;
                        ring[p] = ImVec2(foot.x + cosf(a) * (rx + expand), foot.y + sinf(a) * (ry + expand * 0.34f));
                    }
                    const ImU32 glow = (layer & 1) ? IM_COL32(145, 95, 255, 72) : IM_COL32(70, 225, 255, 90);
                    draw->AddPolyline(ring, pts, glow, ImDrawFlags_Closed, layer == 0 ? 2.0f * scale : 1.2f * scale);
                }
                for (int sp = 0; sp < 4; ++sp) {
                    const float a = now * (1.1f + 0.08f * sp) + 6.28318530718f * (float)sp / 4.0f;
                    const float blink = 0.5f + 0.5f * sinf(now * 6.0f + sp * 1.7f);
                    const ImVec2 q(foot.x + cosf(a) * (rx + 4.0f), foot.y + sinf(a) * (ry + 2.0f));
                    draw->AddCircleFilled(q, 1.5f + 1.8f * blink, IM_COL32(238, 252, 255, (int)(140 + 110 * blink)), 10);
                }
            }
        }

        // Health is optional and isolated: a missing health component does not hide the rest of ESP.
        if (checkboxStates["ESP HEALTH"] && Class_AttackableTarget__m_AttackableInfo &&
            Class_AttackableTarget__m_Health && Class_AttackableTarget__m_MaxHealth) {
            uintptr_t attackable = 0;
            if (StretchESPRead((uintptr_t)pawn + Class_AttackableTarget__m_AttackableInfo, attackable) &&
                StretchESPIsReadable(reinterpret_cast<void*>(attackable), sizeof(void*))) {
                float currentHp = 0.0f, maxHp = 0.0f;
                if (StretchESPRead(attackable + Class_AttackableTarget__m_Health, currentHp) &&
                    StretchESPRead(attackable + Class_AttackableTarget__m_MaxHealth, maxHp) &&
                    std::isfinite(currentHp) && std::isfinite(maxHp) && maxHp > 0.0f) {
                    const float ratio = std::max(0.0f, std::min(1.0f, currentHp / maxHp));
                    const ImU32 hpColor = IM_COL32((int)(255.0f * (1.0f - ratio)), (int)(255.0f * ratio), 36, 245);
                    const float barW = std::max(28.0f, boxWidth * 1.2f);
                    const float barH = std::max(3.0f, 4.0f * scale);
                    const ImVec2 barMin(headX - barW * 0.5f, topLeft.y - barH - 5.0f * scale);
                    const ImVec2 barMax(barMin.x + barW, barMin.y + barH);
                    draw->AddRectFilled(barMin, barMax, IM_COL32(8, 8, 8, 210), barH * 0.35f);
                    if (ratio > 0.0f) draw->AddRectFilled(barMin, ImVec2(barMin.x + barW * ratio, barMax.y), hpColor, barH * 0.35f);
                    draw->AddRect(barMin, barMax, IM_COL32(245, 245, 245, 175), barH * 0.35f, 0, 1.0f * scale);
                }
            }
        }

        // Name label: read metadata only when its field and string are mapped/readable.
        if (checkboxStates["ESP NAME"] && Class_Pawn__m_PlayerInfo && Class_PlayerInfo__m_NickName) {
            uintptr_t playerInfo = 0;
            String* nick = nullptr;
            if (StretchESPRead((uintptr_t)pawn + Class_Pawn__m_PlayerInfo, playerInfo) &&
                StretchESPIsReadable(reinterpret_cast<void*>(playerInfo), sizeof(void*)) &&
                StretchESPRead(playerInfo + Class_PlayerInfo__m_NickName, nick) &&
                StretchESPIsReadable(nick, offsetof(String, start_char) + sizeof(uint16_t))) {
                char safeName[96] = {0};
                if (StretchESPCopyString(nick, safeName, sizeof(safeName))) {
                    ImVec2 textSize = ImGui::CalcTextSize(safeName);
                    const ImU32 nameColor = isBot ? IM_COL32(70, 255, 115, 255) : IM_COL32(255, 245, 135, 255);
                    draw->AddText(nullptr, fontSize, ImVec2(headX - textSize.x * 0.5f, topLeft.y - fontSize - 6.0f * scale), nameColor, safeName);
                }
            }
        }

        if (checkboxStates["ESP DISTANCE"]) {
            char distanceText[32];
            std::snprintf(distanceText, sizeof(distanceText), "%dm", (int)distance);
            ImVec2 textSize = ImGui::CalcTextSize(distanceText);
            draw->AddText(nullptr, fontSize, ImVec2(headX - textSize.x * 0.5f, bottomRight.y + 2.0f * scale), IM_COL32(245, 245, 245, 240), distanceText);
        }

        if (checkboxStates["ESP PLAYER ID"] && Class_Pawn_get_TeamSeatId &&
            StretchESPIsReadable(reinterpret_cast<void*>(Class_Pawn_get_TeamSeatId), sizeof(void*))) {
            unsigned int* idPtr = pawn->get_TeamSeatId();
            unsigned int teamId = 0;
            if (StretchESPRead((uintptr_t)idPtr, teamId)) {
                char idText[32];
                std::snprintf(idText, sizeof(idText), "ID %u", teamId);
                draw->AddText(nullptr, fontSize * 0.9f, ImVec2(bottomRight.x + 3.0f * scale, topLeft.y), IM_COL32(255,255,255,230), idText);
            }
        }
    }

    // Cyrus V4-style compact bot/player counter. It uses the current source's ESP COUNT toggle.
    if (checkboxStates["ESP COUNT"]) {
        ImDrawList* hud = ImGui::GetForegroundDrawList();
        if (hud) {
            const float w = 94.0f * scale, h = 30.0f * scale, gap = 3.0f * scale;
            const float x = glWidth * 0.5f, y = 28.0f * scale;
            const ImVec2 botMin(x - w - gap * 0.5f, y), botMax(x - gap * 0.5f, y + h);
            const ImVec2 playerMin(x + gap * 0.5f, y), playerMax(x + w + gap * 0.5f, y + h);
            hud->AddRectFilled(botMin, botMax, IM_COL32(220, 48, 38, 238), 7.0f * scale);
            hud->AddRectFilled(playerMin, playerMax, IM_COL32(48, 174, 83, 238), 7.0f * scale);
            hud->AddRect(botMin, botMax, IM_COL32(255,255,255,95), 7.0f * scale, 0, 1.0f * scale);
            hud->AddRect(playerMin, playerMax, IM_COL32(255,255,255,95), 7.0f * scale, 0, 1.0f * scale);
            char botsText[24], playersText[24];
            std::snprintf(botsText, sizeof(botsText), "BOTS  %d", totalBots);
            std::snprintf(playersText, sizeof(playersText), "PLAYERS  %d", totalPlayers);
            hud->AddText(nullptr, fontSize * 0.85f, ImVec2(botMin.x + 8.0f * scale, botMin.y + 7.0f * scale), IM_COL32_WHITE, botsText);
            hud->AddText(nullptr, fontSize * 0.85f, ImVec2(playerMin.x + 8.0f * scale, playerMin.y + 7.0f * scale), IM_COL32_WHITE, playersText);
        }
    }
}

void DrawESP(ImDrawList *draw) {
    drawESP__Player_CyrusStyle(draw);
}

