#pragma once

uintptr_t m_Anogs;
int32_t sWidth = 0;
int32_t sHeight = 0;
void *BaseWorld_Instance = 0;


enum TouchType {
    Direct = 0,
    Indirect = 1,
    Stylus = 2,
};

struct Touch {
    int m_FingerId;
    Vector2 m_Position;
    Vector2 m_RawPosition;
    Vector2 m_PositionDelta;
    float m_TimeDelta;
    int m_TapCount;
    TouchPhase m_Phase;
    TouchType m_Type;
    float m_Pressure;
    float m_maximumPossiblePressure;
    float m_Radius;
    float m_RadiusVariance;
    float m_AltitudeAngle;
    float m_AzimuthAngle;
};

enum LineTarget {
    Top = 0,
    Center = 1,
    Bottom = 2
};

enum CrosshairTarget {
    Circle = 0,
    Plus = 1,
    Ekis = 2
};

enum EAim {
  Distance = 0,
  Crosshair = 1
};

enum EAimTarget {
    Heads = 0,
    Chests = 1,
    Body = 2
};

enum EAimTrigger {
    None = 0,
    Shooting = 1,
    Scoping = 2
};

enum EBulletTrack {
    Bt = 0,
    Bt1 = 1,
    Bt2 = 2,
};

struct sConfig {
    struct sInitImGui {
        bool g_Initialized;
        bool clearMousePos = true;
        uintptr_t thiz;
    };
    sInitImGui ImGuiMenu{0};
    
struct sWeaponAim {
        bool Aimbot360;
        bool AimLock;
        bool AimAssist;
        bool showAimAssistSlider;
        float AimAssistSize;
        bool AimSilent;
        bool AimSilentTools;
        bool AimSilentSettings;
        EAimTarget Target;
        EAimTrigger Trigger;
        EAim By;
        float size;
        bool BulletTrack;
        bool BulletTrackSettings;
        EBulletTrack Options;
    };
    sWeaponAim Aim{0};
    
    
    struct sESPMenuLineScale {
        bool ScaleLine;
        bool showLineSlider;
        float lineSize;
    };
    sESPMenuLineScale sESPMenuLineScale{0};
    
    struct sESPMenuNameScale {
        bool ScaleName;
        bool showNameSlider;
        float NameSize;
    };
    sESPMenuNameScale sESPMenuNameScale{0};
    
    struct sESPMenuDistanceScale {
        bool ScaleDistance;
        bool showDistanceSlider;
        float DistanceSize;
    };
    sESPMenuDistanceScale sESPMenuDistanceScale{0};
    
    struct sMAINMenu {
        bool LOBBY;
    };
    sMAINMenu sMAINMenu{0};
    
    struct sESPMenu {
        bool ESP;
        bool Alert;
        bool Count;
        bool Name;
        bool isPlayerLine;
        LineTarget Target;
        CrosshairTarget CrosshairType;
        bool Box;
        bool Health;
        bool Distance;
        bool Skelton;
        bool Crosshair;
        bool Aimline;
    };
    sESPMenu ESPMenu{0};
    
    struct sColorsESPPLAYER {
        float *LinePLAYER;
        float *BoxPLAYER;
        float *NamePLAYER;
        float *HealthPLAYER;
        float *DistancePLAYER;
    };
    sColorsESPPLAYER sColorsESPPLAYER{0};
    
    struct sColorsESPBOT {
        float *LineBOT;
        float *BoxBOT;
        float *NameBOT;
        float *HealthBOT;
        float *DistanceBOT;
    };
    sColorsESPBOT sColorsESPBOT{0};
    
    struct sColorsESPOTHERS {
        float *PovOTHERS;
    };
    sColorsESPOTHERS sColorsESPOTHERS{0};
    
    struct sExtraMenu {
        bool Test;
        bool Test1;
        float TestFloat;
        bool RadarMap;
        bool Recoil;
        bool Reload;
        bool AdvanceUav;
        bool Small;
        bool Aimbot;
        bool HighJump;
        bool FastScope;
        bool Overheat;
        bool AutoFire;
        bool InstantFire;
        bool NoShake;
        bool Radar;
        bool NoSpread;
        bool NoSmoke;
        bool RapidFire;
        bool FastSwitch;
        bool NoParachute;
        bool RedLight;
        bool GravityFall;
        bool Hitbox;
        bool WallHack;
        bool DarkMode;
        bool SpectatorTag;
        bool WeaponRange;
        bool NoOverheat;
		bool Cb;
        bool StretchScreen;
        float StretchAspect;
		
    };
    sExtraMenu ExtraMenu{0};
    
    struct sExtraMenu1 {
        bool Jump;
        bool showJumpSlider;
        float jumpHeight;
        bool Swim;
        bool showSwimSlider;
        float swimSpeed;
        bool Slide;
        bool showSlideSlider;
        float slideSpeed;
        bool Flash;
        bool showFlashSlider;
        float movementSpeed;
        bool Crouch;
        bool showCrouchSlider;
        float crouchSpeed;
        bool Prone;
        bool showProneSlider;
        float proneSpeed;
    };
    sExtraMenu1 ExtraMenu1{0};
    
};

 sConfig Config{0};


