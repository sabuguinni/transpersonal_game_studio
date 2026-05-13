#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "TimerManager.h"
#include "Audio_ScreenShakeController.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    None        UMETA(DisplayName = "None"),
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Heavy       UMETA(DisplayName = "Heavy"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct FAudio_ShakeProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float FalloffRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    bool bUseFalloff = true;

    FAudio_ShakeProfile()
    {
        Amplitude = 1.0f;
        Frequency = 10.0f;
        Duration = 1.0f;
        FalloffRadius = 1000.0f;
        bUseFalloff = true;
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

public:
    virtual void Tick(float DeltaTime) override;

    // Screen shake triggering
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerScreenShake(EAudio_ShakeIntensity Intensity, FVector SourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeProfile& ShakeProfile, FVector SourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerTRexFootstep(FVector TRexLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerDinosaurRoar(FVector DinosaurLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerEnvironmentalShake(FVector SourceLocation, float Magnitude);

    // Proximity-based shake system
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void StartProximityShake(AActor* TargetActor, float UpdateInterval = 0.1f);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void StopProximityShake();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profiles")
    TMap<EAudio_ShakeIntensity, FAudio_ShakeProfile> ShakeProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float GlobalShakeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxShakeDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableScreenShake = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Settings")
    float TRexShakeRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Settings")
    float TRexShakeIntensity = 3.0f;

protected:
    // Camera shake source component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCameraShakeSourceComponent* CameraShakeSource;

    // Audio component for shake-related sounds
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    // Proximity tracking
    UPROPERTY()
    AActor* TrackedActor;

    FTimerHandle ProximityTimerHandle;
    float LastProximityDistance;

    // Internal methods
    void UpdateProximityShake();
    float CalculateShakeIntensity(float Distance, float MaxDistance) const;
    void ApplyScreenShake(const FAudio_ShakeProfile& Profile, FVector Location);

private:
    void InitializeShakeProfiles();
    bool IsPlayerNearby(FVector Location, float Radius) const;
};