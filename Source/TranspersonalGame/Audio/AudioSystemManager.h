#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/AmbientSound.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    VolcanicField   UMETA(DisplayName = "Volcanic Field"),
    NightCampfire   UMETA(DisplayName = "Night Campfire")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe — No Predators"),
    Aware       UMETA(DisplayName = "Aware — Predator Nearby"),
    Danger      UMETA(DisplayName = "Danger — Predator Engaged"),
    Critical    UMETA(DisplayName = "Critical — Under Attack")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ReverbAmount = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MusicIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bNocturnalSounds = false;
};

USTRUCT(BlueprintType)
struct FAudio_ThreatState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    EAudio_ThreatLevel ThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    float ThreatProximity = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    FString PredatorSpecies = TEXT("None");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    float MusicTransitionSpeed = 2.0f;
};

/**
 * UAudio_SystemManager
 * Adaptive audio manager for the prehistoric survival game.
 * Controls ambient soundscapes, music intensity, and threat-reactive audio.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Zone management
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetBiomeZone(EAudio_BiomeZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_BiomeZone GetCurrentBiomeZone() const { return CurrentZoneConfig.BiomeZone; }

    // Threat-reactive music
    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void UpdateThreatLevel(EAudio_ThreatLevel NewThreat, float Proximity, const FString& Species);

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatState.ThreatLevel; }

    // Day/night audio transitions
    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    // Screen shake trigger (called by dinosaur proximity system)
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerFootstepRumble(float Intensity, float Duration);

    // Campfire audio
    UFUNCTION(BlueprintCallable, Category = "Audio|Campfire")
    void SetCampfireActive(bool bActive);

    UFUNCTION(BlueprintPure, Category = "Audio|Campfire")
    bool IsCampfireActive() const { return bCampfireActive; }

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    FAudio_ZoneConfig CurrentZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    FAudio_ThreatState CurrentThreatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.8f;

private:
    float CurrentTimeOfDay = 0.5f;
    bool bCampfireActive = false;
    float ThreatTransitionAlpha = 0.0f;

    void UpdateMusicIntensity(float DeltaTime);
    void UpdateAmbientLayers(float DeltaTime);
    float GetMusicTargetIntensity() const;
};
