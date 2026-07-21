// AmbientAudioManager.h
// Agent #16 — Audio Agent — PROD_CYCLE_AUTO_20260620_008
// Manages spatial ambient audio zones for MinPlayableMap.
// Zones: camp fire, night insects, raptor valley, wind, T-Rex rumble.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "AmbientAudioManager.generated.h"

// Enum: audio zone types present in the prehistoric world
UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    CampFire        UMETA(DisplayName = "Camp Fire"),
    NightInsects    UMETA(DisplayName = "Night Insects"),
    RaptorValley    UMETA(DisplayName = "Raptor Valley"),
    WindGlobal      UMETA(DisplayName = "Wind Global"),
    TRexRumble      UMETA(DisplayName = "T-Rex Rumble"),
    WaterSource     UMETA(DisplayName = "Water Source"),
    Custom          UMETA(DisplayName = "Custom")
};

// Struct: defines a single ambient audio zone
USTRUCT(BlueprintType)
struct FAudio_AmbientZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Custom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FVector WorldPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float InnerRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float OuterRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BaseVolume = 1.0f;

    // If true, volume scales with time-of-day (night = louder)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bNightAmplified = false;

    // If true, pitch shifts when player is in danger (health < 50%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bDangerPitchShift = false;
};

/**
 * AAudio_AmbientManager
 * Placed once in the level. Owns all ambient audio components.
 * Drives volume/pitch based on player proximity, time-of-day, and survival state.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AmbientManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Zone Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zones")
    TArray<FAudio_AmbientZone> AmbientZones;

    // --- Runtime State ---

    // Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Time")
    float TimeOfDay = 0.25f;

    // Player health fraction (0.0 = dead, 1.0 = full)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Player")
    float PlayerHealthFraction = 1.0f;

    // Distance to nearest T-Rex (updated each tick)
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Danger",
        meta = (AllowPrivateAccess = "true"))
    float NearestTRexDistance = 99999.0f;

    // --- Blueprint Events ---

    // Called when player enters a raptor zone (triggers danger music)
    UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Events")
    void OnPlayerEnterRaptorZone();

    // Called when T-Rex is within 800 units
    UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Events")
    void OnTRexProximityAlert(float Distance);

    // Called at dawn (TimeOfDay crosses 0.2)
    UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Events")
    void OnDawnTransition();

    // Called at dusk (TimeOfDay crosses 0.75)
    UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Events")
    void OnDuskTransition();

    // --- Blueprint Callable ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Control")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio|Control")
    void SetPlayerHealthFraction(float HealthFraction);

    UFUNCTION(BlueprintCallable, Category = "Audio|Control")
    void UpdateTRexDistance(float Distance);

    // Returns volume multiplier for a zone given current world state
    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    float GetZoneVolumeMultiplier(EAudio_ZoneType ZoneType) const;

    // Returns pitch multiplier for a zone given current world state
    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    float GetZonePitchMultiplier(EAudio_ZoneType ZoneType) const;

private:
    // Cached audio components per zone (index matches AmbientZones)
    UPROPERTY()
    TArray<UAudioComponent*> AudioComponents;

    // Previous time of day — for dawn/dusk event detection
    float PreviousTimeOfDay = 0.25f;

    // Whether T-Rex alert was already fired this proximity window
    bool bTRexAlertFired = false;

    void UpdateAllZoneVolumes();
    void CheckTimeTransitions();
    void CheckTRexProximity();
};
