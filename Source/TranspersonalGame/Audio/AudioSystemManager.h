#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager
// Manages adaptive ambient audio, dinosaur proximity audio,
// day/night audio transitions, and screen shake triggers.
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    DinosaurProximity   UMETA(DisplayName = "Dinosaur Proximity"),
    RiverAmbient        UMETA(DisplayName = "River Ambient"),
    WindGust            UMETA(DisplayName = "Wind Gust"),
    CampfireNight       UMETA(DisplayName = "Campfire Night"),
    ForestAmbient       UMETA(DisplayName = "Forest Ambient"),
    DangerAlert         UMETA(DisplayName = "Danger Alert"),
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::ForestAmbient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TriggerRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bTriggersScreenShake = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ScreenShakeIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_DinoSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName DinoSpecies = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepShakeRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepShakeIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RoarAudibleRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BreathingAudibleRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;
};

// ============================================================
// AAudio_ZoneTrigger — placed in level to define audio zones
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ZoneTrigger : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneTrigger();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bActiveAtDawn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bActiveAtDay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bActiveAtDusk = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bActiveAtNight = true;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetZoneActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsActiveForTimeOfDay(EAudio_TimeOfDay TimeOfDay) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetVolumeForDistance(float Distance) const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    UAudioComponent* AudioComp;

    bool bCurrentlyActive = false;
};

// ============================================================
// UAudio_AdaptiveManager — subsystem managing all audio zones
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_AdaptiveManager : public UObject
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DayNightCycleHours = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_DinoSoundProfile> DinoProfiles;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateTimeOfDay(float GameHour);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterDinoProximity(FName DinoSpecies, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerDangerAlert(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetAmbientVolumeMultiplier() const;

private:
    float CurrentDangerLevel = 0.0f;
    float LastDinoProximityDistance = 9999.0f;
};
