#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Audio_ScreenShakeController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    float Range = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    bool bFadeIn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    bool bFadeOut = true;

    FAudio_ShakeParameters()
    {
        Intensity = 1.0f;
        Duration = 2.0f;
        Range = 1000.0f;
        bFadeIn = true;
        bFadeOut = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximityShakeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Shake")
    float MinDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Shake")
    float MaxDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Shake")
    float MinIntensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Shake")
    float MaxIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Shake")
    bool bUseHeartbeat = true;

    FAudio_ProximityShakeData()
    {
        MinDistance = 100.0f;
        MaxDistance = 2000.0f;
        MinIntensity = 0.1f;
        MaxIntensity = 5.0f;
        bUseHeartbeat = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ScreenShakeController : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ScreenShakeController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core shake parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake", meta = (AllowPrivateAccess = "true"))
    FAudio_ShakeParameters TRexShakeParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake", meta = (AllowPrivateAccess = "true"))
    FAudio_ShakeParameters RaptorShakeParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake", meta = (AllowPrivateAccess = "true"))
    FAudio_ShakeParameters VolcanicShakeParams;

    // Proximity-based shake system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Shake", meta = (AllowPrivateAccess = "true"))
    FAudio_ProximityShakeData ProximityData;

    // Audio components for shake feedback
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* HeartbeatAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* RumbleAudioComponent;

    // Sound assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets", meta = (AllowPrivateAccess = "true"))
    class USoundCue* HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets", meta = (AllowPrivateAccess = "true"))
    class USoundCue* RumbleSound;

    // Shake state tracking
    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    bool bIsShaking;

    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    float CurrentShakeIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    float ShakeTimer;

public:
    // Main shake trigger functions
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerTRexShake(const FVector& TRexLocation, float TRexSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerRaptorShake(const FVector& RaptorLocation, int32 RaptorCount = 1);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerVolcanicShake(float VolcanicIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeParameters& ShakeParams, const FVector& SourceLocation);

    // Proximity-based shake system
    UFUNCTION(BlueprintCallable, Category = "Proximity Shake")
    void UpdateProximityShake(const TArray<AActor*>& NearbyThreats);

    UFUNCTION(BlueprintCallable, Category = "Proximity Shake")
    void StartProximityShake(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Proximity Shake")
    void StopProximityShake();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void StopAllShakes();

    UFUNCTION(BlueprintPure, Category = "Screen Shake")
    float CalculateShakeIntensity(const FVector& SourceLocation, float BaseIntensity, float MaxRange) const;

    UFUNCTION(BlueprintPure, Category = "Screen Shake")
    bool IsPlayerInRange(const FVector& SourceLocation, float Range) const;

private:
    void InitializeAudioComponents();
    void UpdateHeartbeatAudio(float Intensity);
    void UpdateRumbleAudio(float Intensity);
    float CalculateDistanceBasedIntensity(const FVector& SourceLocation) const;
    
    // Internal shake management
    void ExecuteShake(const FAudio_ShakeParameters& ShakeParams, const FVector& SourceLocation);
    class APlayerController* GetPlayerController() const;
    class APawn* GetPlayerPawn() const;
};