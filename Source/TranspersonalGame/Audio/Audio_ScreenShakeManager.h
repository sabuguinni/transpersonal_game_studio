#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"
#include "Audio_ScreenShakeManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    None = 0,
    Light = 1,
    Medium = 2,
    Heavy = 3,
    Extreme = 4
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    bool bFadeIn = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    bool bFadeOut = true;

    FAudio_ShakeProfile()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        bFadeIn = false;
        bFadeOut = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenShakeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeManager();

protected:
    virtual void BeginPlay() override;

    // Shake profiles for different intensities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    TMap<EAudio_ShakeIntensity, FAudio_ShakeProfile> ShakeProfiles;

    // Distance-based shake parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Shake")
    float MaxShakeDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Shake")
    float MinShakeDistance = 100.0f;

    // T-Rex specific shake settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Shake")
    float TRexFootstepInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Shake")
    float TRexShakeRadius = 1500.0f;

    // Current shake state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsShaking = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentShakeIntensity = 0.0f;

    // Timer for footstep shakes
    FTimerHandle FootstepShakeTimer;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main shake functions
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerShake(EAudio_ShakeIntensity Intensity, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerProximityShake(const FVector& SourceLocation, float SourceMass = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerTRexFootstep(const FVector& FootstepLocation);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void StartTRexProximityShaking(AActor* TRexActor);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void StopTRexProximityShaking();

    // Damage-related shakes
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDamageShake(float DamageAmount, const FVector& DamageDirection);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void StopAllShakes();

    UFUNCTION(BlueprintPure, Category = "Screen Shake")
    float CalculateShakeIntensity(float Distance, float MaxDistance, float MinDistance) const;

    UFUNCTION(BlueprintPure, Category = "Screen Shake")
    EAudio_ShakeIntensity GetShakeIntensityFromDistance(float Distance) const;

protected:
    // Internal shake execution
    void ExecuteShake(const FAudio_ShakeProfile& Profile, float Scale = 1.0f);

    // Timer callback for T-Rex footsteps
    UFUNCTION()
    void OnTRexFootstepTimer();

    // Get player controller for shake execution
    APlayerController* GetPlayerController() const;

    // Current T-Rex being tracked
    UPROPERTY()
    AActor* TrackedTRex = nullptr;

    // Initialize default shake profiles
    void InitializeShakeProfiles();
};