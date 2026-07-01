#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    Plains      UMETA(DisplayName = "Open Plains"),
    Forest      UMETA(DisplayName = "Dense Forest"),
    River       UMETA(DisplayName = "River / Water"),
    Canyon      UMETA(DisplayName = "Canyon / Cave"),
    Volcano     UMETA(DisplayName = "Volcanic Zone"),
    Night       UMETA(DisplayName = "Night Transition")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe — No Predators"),
    Aware       UMETA(DisplayName = "Aware — Distant Threat"),
    Danger      UMETA(DisplayName = "Danger — Predator Nearby"),
    Combat      UMETA(DisplayName = "Combat — Active Attack"),
    Flee        UMETA(DisplayName = "Flee — Run for Life")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MusicVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ReverbAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bEnableInsects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bEnableWind = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bEnableWater = false;
};

USTRUCT(BlueprintType)
struct FAudio_ThreatState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    EAudio_ThreatLevel ThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    float ThreatDistance = 9999.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    float HeartbeatRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    bool bTRexProximity = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Threat")
    bool bRaptorProximity = false;
};

/**
 * UAudio_SystemManager
 * Adaptive audio manager for the prehistoric survival game.
 * Controls ambient zones, threat-reactive music, and environmental SFX.
 * Integrates with MetaSounds for dynamic audio blending.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Zone Management ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetBiomeZone(EAudio_BiomeZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_BiomeZone GetCurrentBiomeZone() const { return CurrentZoneConfig.BiomeZone; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void BlendToZone(EAudio_BiomeZone TargetZone, float BlendTime = 3.0f);

    // --- Threat System ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void UpdateThreatState(const FAudio_ThreatState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreat.ThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void TriggerTRexRoar(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void TriggerRaptorCall(int32 PackSize);

    // --- Day/Night Audio ---
    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void OnDawnTransition();

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void OnDuskTransition();

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void OnNightTransition();

    // --- Survival Audio ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayFootstep(bool bIsRunning, bool bOnWater);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayCraftingSound(FName ItemType);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayHungerWarning();

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayThirstWarning();

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayDamageImpact(float DamageAmount);

    // --- Screen Shake (T-Rex proximity) ---
    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerTRexFootstepShake(float Intensity);

    // --- Volume Control ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void SetAmbientVolume(float Volume);

    // --- Properties ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    FAudio_ZoneConfig CurrentZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    FAudio_ThreatState CurrentThreat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float CurrentTimeOfDay = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    bool bIsNight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float ZoneBlendAlpha = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float ThreatMusicIntensity = 0.0f;

private:
    void UpdateAmbientBlend(float DeltaTime);
    void UpdateThreatMusic(float DeltaTime);
    void UpdateHeartbeat(float DeltaTime);

    EAudio_BiomeZone PreviousZone = EAudio_BiomeZone::Plains;
    float ZoneBlendTime = 3.0f;
    float ZoneBlendElapsed = 0.0f;
    bool bIsBlending = false;
    float HeartbeatTimer = 0.0f;
    float FootstepTimer = 0.0f;
};
