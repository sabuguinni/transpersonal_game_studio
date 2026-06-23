#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Enums — Audio zones and states for adaptive music
// ============================================================

UENUM(BlueprintType)
enum class EAudio_Zone : uint8
{
    Safe        UMETA(DisplayName = "Safe Zone"),
    Exploration UMETA(DisplayName = "Exploration"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat"),
    Night       UMETA(DisplayName = "Night"),
    Storm       UMETA(DisplayName = "Storm")
};

UENUM(BlueprintType)
enum class EAudio_DinoSize : uint8
{
    Small   UMETA(DisplayName = "Small"),   // Raptor-class
    Medium  UMETA(DisplayName = "Medium"),  // Carnotaurus-class
    Large   UMETA(DisplayName = "Large")    // T-Rex / Brachiosaurus-class
};

// ============================================================
// Structs
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* SoundAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_Zone AssociatedZone = EAudio_Zone::Exploration;
};

USTRUCT(BlueprintType)
struct FAudio_DinoFootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_DinoSize DinoSize = EAudio_DinoSize::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float StepVolumeMultiplier = 1.0f;

    // Radius in cm within which the player hears ground rumble
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RumbleRadius = 1500.0f;

    // Screen shake intensity multiplier (0=none, 1=full)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ScreenShakeIntensity = 0.5f;
};

USTRUCT(BlueprintType)
struct FAudio_QuestSting
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* StartSting = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* CompleteSting = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float StingVolume = 1.0f;
};

// ============================================================
// Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAudio_OnZoneChanged, EAudio_Zone, NewZone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAudio_OnDinoNearby, EAudio_DinoSize, DinoSize, float, Distance);

// ============================================================
// UAudioSystemManager — GameInstance Subsystem
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    // --- Subsystem lifecycle ---
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Zone management ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    void SetAudioZone(EAudio_Zone NewZone);

    UFUNCTION(BlueprintPure, Category = "Audio|Zones")
    EAudio_Zone GetCurrentAudioZone() const { return CurrentZone; }

    // --- Ambient layers ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void PlayAmbientLayer(const FAudio_AmbientLayer& Layer);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void StopAllAmbientLayers(float FadeOutTime = 2.0f);

    // --- Campfire audio ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void PlayCampfireAudio(FVector WorldLocation, float Radius = 400.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void StopCampfireAudio();

    // --- Dinosaur proximity audio ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void NotifyDinoFootstep(EAudio_DinoSize DinoSize, FVector StepLocation, AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinoVocalization(EAudio_DinoSize DinoSize, FVector DinoLocation);

    // --- Quest audio stings ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Quest")
    void PlayQuestStartSting(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Quest")
    void PlayQuestCompleteSting(FName QuestID);

    // --- Survival state audio ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void UpdateSurvivalAudio(float HealthNormalized, float StaminaNormalized, float FearNormalized);

    // --- Day/Night audio transition ---
    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void TransitionToNightAudio(float TransitionDuration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void TransitionToDayAudio(float TransitionDuration = 10.0f);

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnZoneChanged OnZoneChanged;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnDinoNearby OnDinoNearby;

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_DinoFootstepConfig> DinoFootstepConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_QuestSting> QuestStings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.9f;

private:

    UPROPERTY()
    EAudio_Zone CurrentZone = EAudio_Zone::Exploration;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAmbientComponents;

    UPROPERTY()
    UAudioComponent* CampfireComponent = nullptr;

    // Freesound asset references (populated at runtime via config)
    // Campfire: Freesound ID 620324 — "Campfire crackling - Loop"
    // Raptor:   Freesound ID 0 — no results, use procedural tension
    // Wind:     Freesound ID 0 — no results, use procedural wind
    static const int32 FreesoundCampfireID = 620324;
    static const int32 FreesoundFireplaceID = 852107;
};
