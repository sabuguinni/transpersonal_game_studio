#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Audio_GameFeedbackSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_FeedbackType : uint8
{
    None UMETA(DisplayName = "None"),
    TRexProximity UMETA(DisplayName = "T-Rex Proximity"),
    RaptorPack UMETA(DisplayName = "Raptor Pack"),
    LowHealth UMETA(DisplayName = "Low Health"),
    LowHunger UMETA(DisplayName = "Low Hunger"),
    LowThirst UMETA(DisplayName = "Low Thirst"),
    HighFear UMETA(DisplayName = "High Fear"),
    SafeZone UMETA(DisplayName = "Safe Zone"),
    DangerZone UMETA(DisplayName = "Danger Zone"),
    WaterNearby UMETA(DisplayName = "Water Nearby"),
    FoodFound UMETA(DisplayName = "Food Found")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FeedbackConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    EAudio_FeedbackType FeedbackType = EAudio_FeedbackType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    TSoftObjectPtr<USoundCue> AudioCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float TriggerDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float Cooldown = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    bool bSpatial = true;

    FAudio_FeedbackConfig()
    {
        FeedbackType = EAudio_FeedbackType::None;
        Volume = 1.0f;
        Pitch = 1.0f;
        TriggerDistance = 1000.0f;
        Cooldown = 5.0f;
        bSpatial = true;
    }
};

/**
 * Audio Game Feedback System
 * Provides contextual audio feedback based on player state and environment
 * Integrates with survival stats and proximity detection
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_GameFeedbackSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_GameFeedbackSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Main feedback trigger
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerFeedback(EAudio_FeedbackType FeedbackType, FVector Location = FVector::ZeroVector);

    // Proximity-based feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void CheckProximityFeedback(AActor* TargetActor, EAudio_FeedbackType FeedbackType);

    // Survival state feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void CheckSurvivalFeedback(float Health, float Hunger, float Thirst, float Fear);

    // Environmental feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void CheckEnvironmentalFeedback();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void SetFeedbackConfig(EAudio_FeedbackType FeedbackType, const FAudio_FeedbackConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    FAudio_FeedbackConfig GetFeedbackConfig(EAudio_FeedbackType FeedbackType) const;

    // Control
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void EnableFeedback(bool bEnable) { bFeedbackEnabled = bEnable; }

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    bool IsFeedbackEnabled() const { return bFeedbackEnabled; }

protected:
    // Configuration map
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    TMap<EAudio_FeedbackType, FAudio_FeedbackConfig> FeedbackConfigs;

    // Audio component for playback
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Feedback")
    TObjectPtr<UAudioComponent> AudioComponent;

    // State tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Feedback")
    bool bFeedbackEnabled = true;

    // Cooldown tracking
    UPROPERTY()
    TMap<EAudio_FeedbackType, float> FeedbackCooldowns;

    // Internal methods
    void InitializeFeedbackConfigs();
    bool CanTriggerFeedback(EAudio_FeedbackType FeedbackType) const;
    void PlayFeedbackAudio(const FAudio_FeedbackConfig& Config, FVector Location);
    void UpdateCooldowns(float DeltaTime);

    // Default configurations
    void SetupDefaultConfigs();
    
    // Player reference
    UPROPERTY()
    TObjectPtr<APawn> PlayerPawn;

    // Cached world reference
    UPROPERTY()
    TObjectPtr<UWorld> CachedWorld;
};