#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_AdvancedFeedbackController.generated.h"

UENUM(BlueprintType)
enum class EAudio_FeedbackType : uint8
{
    None            UMETA(DisplayName = "None"),
    TRexProximity   UMETA(DisplayName = "T-Rex Proximity"),
    DamageReceived  UMETA(DisplayName = "Damage Received"),
    EnvironmentalHazard UMETA(DisplayName = "Environmental Hazard"),
    CriticalAlert   UMETA(DisplayName = "Critical Alert"),
    SeismicActivity UMETA(DisplayName = "Seismic Activity")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FeedbackConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
    EAudio_FeedbackType FeedbackType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
    float IntensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
    bool bEnableScreenShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
    bool bEnableAudioCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
    bool bEnableVisualEffect;

    FAudio_FeedbackConfig()
    {
        FeedbackType = EAudio_FeedbackType::None;
        IntensityLevel = 1.0f;
        Duration = 2.0f;
        bEnableScreenShake = true;
        bEnableAudioCue = true;
        bEnableVisualEffect = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AdvancedFeedbackController : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AdvancedFeedbackController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback Settings")
    TMap<EAudio_FeedbackType, FAudio_FeedbackConfig> FeedbackConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundBase* TRexProximitySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundBase* DamageReceivedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundBase* EnvironmentalHazardSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundBase* CriticalAlertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    class USoundBase* SeismicActivitySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
    TSubclassOf<class UCameraShakeBase> TRexProximityCameraShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
    TSubclassOf<class UCameraShakeBase> DamageCameraShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
    TSubclassOf<class UCameraShakeBase> SeismicCameraShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback Settings")
    float MaxFeedbackDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback Settings")
    float FeedbackCooldownTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsFeedbackActive;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_FeedbackType CurrentFeedbackType;

    FTimerHandle FeedbackCooldownTimer;

public:
    UFUNCTION(BlueprintCallable, Category = "Feedback Control")
    void TriggerFeedback(EAudio_FeedbackType FeedbackType, float Intensity = 1.0f, FVector SourceLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Feedback Control")
    void StopAllFeedback();

    UFUNCTION(BlueprintCallable, Category = "Feedback Control")
    void SetFeedbackConfiguration(EAudio_FeedbackType FeedbackType, const FAudio_FeedbackConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Feedback Control")
    bool IsFeedbackTypeActive(EAudio_FeedbackType FeedbackType) const;

    UFUNCTION(BlueprintCallable, Category = "Proximity Detection")
    void CheckTRexProximity();

    UFUNCTION(BlueprintCallable, Category = "Damage System")
    void OnPlayerDamaged(float DamageAmount, FVector DamageLocation);

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void OnEnvironmentalHazard(FVector HazardLocation, float HazardIntensity);

protected:
    UFUNCTION()
    void OnFeedbackCooldownComplete();

    void PlayFeedbackAudio(EAudio_FeedbackType FeedbackType, float Intensity);
    void TriggerCameraShake(EAudio_FeedbackType FeedbackType, float Intensity);
    void InitializeFeedbackConfigurations();
    
    class USoundBase* GetSoundForFeedbackType(EAudio_FeedbackType FeedbackType) const;
    TSubclassOf<class UCameraShakeBase> GetCameraShakeForFeedbackType(EAudio_FeedbackType FeedbackType) const;
    float CalculateIntensityByDistance(FVector SourceLocation, float MaxDistance) const;

public:
    virtual void Tick(float DeltaTime) override;
};