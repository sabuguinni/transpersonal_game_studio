#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerVolume.h"
#include "Sound/SoundCue.h"
#include "AudioFeedbackSystem.generated.h"

// Audio feedback types for different game events
UENUM(BlueprintType)
enum class EAudio_FeedbackType : uint8
{
    None = 0,
    DinosaurProximity,
    PlayerDamage,
    HeartbeatStress,
    EnvironmentalDanger,
    SafeZoneEnter,
    CriticalHealth
};

// Audio intensity levels for dynamic feedback
UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Low = 0,
    Medium,
    High,
    Critical
};

// Struct for audio feedback configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FeedbackConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    EAudio_FeedbackType FeedbackType = EAudio_FeedbackType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    EAudio_IntensityLevel IntensityLevel = EAudio_IntensityLevel::Low;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float FadeInTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float FadeOutTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    bool bLooping = false;

    FAudio_FeedbackConfig()
    {
        FeedbackType = EAudio_FeedbackType::None;
        IntensityLevel = EAudio_IntensityLevel::Low;
        Volume = 1.0f;
        Pitch = 1.0f;
        FadeInTime = 0.5f;
        FadeOutTime = 1.0f;
        bLooping = false;
    }
};

// Proximity audio data for distance-based effects
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float MinDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float MaxDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float VolumeAtMinDistance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float VolumeAtMaxDistance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    UCurveFloat* DistanceCurve = nullptr;

    FAudio_ProximityData()
    {
        MinDistance = 100.0f;
        MaxDistance = 1000.0f;
        VolumeAtMinDistance = 1.0f;
        VolumeAtMaxDistance = 0.1f;
        DistanceCurve = nullptr;
    }
};

/**
 * Audio Feedback System - Manages dynamic audio responses to gameplay events
 * Provides screen shake audio, damage feedback, proximity effects, and environmental audio
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioFeedbackSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudioFeedbackSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* FeedbackAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* ProximityAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* HeartbeatAudioComponent;

    // Audio assets for different feedback types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<EAudio_FeedbackType, USoundCue*> FeedbackSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* ScreenShakeSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* DamageFlashSound;

    // Feedback configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback Config")
    TMap<EAudio_FeedbackType, FAudio_FeedbackConfig> FeedbackConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback Config")
    FAudio_ProximityData ProximityConfig;

    // Current state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_FeedbackType CurrentFeedbackType;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_IntensityLevel CurrentIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentStressLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsPlayingHeartbeat;

    // Timers and state management
    FTimerHandle HeartbeatTimerHandle;
    FTimerHandle FeedbackTimerHandle;

public:
    // Main feedback trigger functions
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerAudioFeedback(EAudio_FeedbackType FeedbackType, EAudio_IntensityLevel Intensity = EAudio_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerScreenShakeAudio(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerDamageFlashAudio(float DamageAmount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerProximityAudio(AActor* SourceActor, float Distance);

    // Stress and heartbeat system
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void UpdateStressLevel(float StressLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void StartHeartbeatAudio(float HeartRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void StopHeartbeatAudio();

    // Environmental audio feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerEnvironmentalFeedback(EAudio_FeedbackType EnvironmentType, const FVector& Location);

    // Configuration and management
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void SetFeedbackConfig(EAudio_FeedbackType FeedbackType, const FAudio_FeedbackConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    FAudio_FeedbackConfig GetFeedbackConfig(EAudio_FeedbackType FeedbackType) const;

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void StopAllFeedback();

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void SetMasterFeedbackVolume(float Volume);

protected:
    // Internal helper functions
    void InitializeFeedbackConfigs();
    void UpdateProximityAudio(float DeltaTime);
    void UpdateHeartbeatRate();
    float CalculateProximityVolume(float Distance) const;
    void PlayFeedbackSound(EAudio_FeedbackType FeedbackType, const FAudio_FeedbackConfig& Config);

    // Timer callbacks
    UFUNCTION()
    void OnHeartbeatTimer();

    UFUNCTION()
    void OnFeedbackComplete();

private:
    // Internal state
    float MasterFeedbackVolume;
    TArray<AActor*> NearbyDinosaurs;
    float LastProximityUpdate;
    float HeartbeatRate;
    bool bInitialized;
};