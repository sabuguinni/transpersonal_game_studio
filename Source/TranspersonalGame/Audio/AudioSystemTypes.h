#pragma once

#include "CoreMinimal.h"
#include "AudioSystemTypes.generated.h"

// ============================================================
// Audio System Types — Agent #16 Audio Agent
// Cycle: PROD_CYCLE_AUTO_20260628_012
// ============================================================

UENUM(BlueprintType)
enum class EAudio_AmbientZone : uint8
{
    Forest          UMETA(DisplayName = "Forest"),
    River           UMETA(DisplayName = "River"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    Cave            UMETA(DisplayName = "Cave"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    Campsite        UMETA(DisplayName = "Campsite"),
    DinosaurTerritory UMETA(DisplayName = "Dinosaur Territory"),
    NightTime       UMETA(DisplayName = "Night Time")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe            UMETA(DisplayName = "Safe"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Danger          UMETA(DisplayName = "Danger"),
    Extreme         UMETA(DisplayName = "Extreme")
};

UENUM(BlueprintType)
enum class EAudio_MusicLayer : uint8
{
    Base            UMETA(DisplayName = "Base Percussion"),
    Tension         UMETA(DisplayName = "Tension Strings"),
    Action          UMETA(DisplayName = "Action Full"),
    Ambient         UMETA(DisplayName = "Ambient Pads"),
    Silence         UMETA(DisplayName = "Silence")
};

USTRUCT(BlueprintType)
struct FAudio_AmbientConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_AmbientZone Zone = EAudio_AmbientZone::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float AttenuationRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 3.0f;
};

USTRUCT(BlueprintType)
struct FAudio_ThreatMusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_ThreatLevel ThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_MusicLayer ActiveLayer = EAudio_MusicLayer::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float TransitionBlendTime = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float DinosaurProximityRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    bool bTRexNearby = false;
};

USTRUCT(BlueprintType)
struct FAudio_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    FName SurfaceType = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float Volume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float PitchVariation = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    bool bSpawnDustParticle = true;
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FName DinosaurSpecies = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float GroundShakeRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float BreathingVolume = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    bool bCausesScreenShake = true;
};
