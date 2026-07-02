#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio_ThreatLevel — adaptive music state based on danger
// ============================================================
UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe — Ambient Only"),
    Aware       UMETA(DisplayName = "Aware — Low Tension"),
    Danger      UMETA(DisplayName = "Danger — High Tension"),
    Combat      UMETA(DisplayName = "Combat — Full Action"),
    Stalked     UMETA(DisplayName = "Stalked — Predator Nearby")
};

// ============================================================
// Audio_TimeOfDay — drives ambient sound layer transitions
// ============================================================
UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

// ============================================================
// FAudio_ScreenShakeConfig — screen shake parameters
// ============================================================
USTRUCT(BlueprintType)
struct FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float Falloff = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float MaxDistance = 2000.0f;
};

// ============================================================
// FAudio_SoundLayer — one ambient layer with volume/fade
// ============================================================
USTRUCT(BlueprintType)
struct FAudio_SoundLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Layer")
    FName LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Layer")
    USoundBase* Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Layer")
    float TargetVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Layer")
    float CurrentVolume = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Layer")
    float FadeSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Layer")
    bool bIsActive = false;
};

// ============================================================
// AAudio_SystemManager — main audio manager actor
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio System Manager"))
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // --- Threat / Adaptive Music ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Adaptive")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Adaptive")
    float ThreatTransitionSpeed = 2.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Adaptive")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Adaptive")
    EAudio_ThreatLevel GetThreatLevel() const { return CurrentThreatLevel; }

    // --- Time of Day Ambient ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    EAudio_TimeOfDay GetTimeOfDay() const { return CurrentTimeOfDay; }

    // --- Screen Shake ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    FAudio_ScreenShakeConfig TRexFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    FAudio_ScreenShakeConfig RaptorAttackShake;

    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerScreenShake(FAudio_ScreenShakeConfig Config, FVector SourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerTRexFootstep(FVector TRexLocation);

    // --- Sound Layers ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Layers")
    TArray<FAudio_SoundLayer> AmbientLayers;

    UFUNCTION(BlueprintCallable, Category = "Audio|Layers")
    void SetLayerVolume(FName LayerName, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Layers")
    void FadeInLayer(FName LayerName, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Layers")
    void FadeOutLayer(FName LayerName, float FadeTime = 2.0f);

    // --- Damage Flash ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float DamageFlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float DamageFlashIntensity = 0.8f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerDamageFlash(float DamageAmount);

    // --- Footstep Dust ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Particles")
    float FootstepDustThreshold = 5.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Particles")
    void SpawnFootstepDust(FVector Location, bool bIsHeavyCreature = false);

    // --- Day/Night Cycle Audio ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float DayNightCycleDuration = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float CurrentDayTime = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void AdvanceDayNightCycle(float DeltaSeconds);

    UFUNCTION(BlueprintPure, Category = "Audio|DayNight")
    float GetDayNightNormalised() const;

private:
    void UpdateAmbientLayers(float DeltaTime);
    void UpdateDayNightAudio();
    EAudio_TimeOfDay CalculateTimeOfDay(float NormalisedTime) const;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    float LastThreatTransitionTime = 0.0f;
    EAudio_ThreatLevel PreviousThreatLevel = EAudio_ThreatLevel::Safe;
};
