#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "TimerManager.h"
#include "Audio_ScreenShakeController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float MaxDistance = 2000.0f;

    FAudio_ShakeSettings()
    {
        Intensity = 1.0f;
        Duration = 0.5f;
        Frequency = 10.0f;
        MaxDistance = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_ScreenShakeController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Screen shake functions
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerTRexProximityShake(FVector TRexLocation, float TRexMass = 7000.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerDamageShake(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerFootstepShake(FVector FootstepLocation, float CreatureMass = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeSettings& ShakeSettings, FVector SourceLocation);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Shake")
    FAudio_ShakeSettings TRexShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Shake")
    FAudio_ShakeSettings DamageShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Shake")
    FAudio_ShakeSettings FootstepShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float TRexDetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float ShakeUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    bool bAutoDetectTRex = true;

private:
    // Internal functions
    void UpdateTRexProximityShake();
    float CalculateShakeIntensity(float Distance, float MaxDistance, float BaseMass) const;
    void ExecuteScreenShake(const FAudio_ShakeSettings& Settings, FVector SourceLocation);
    TArray<AActor*> FindTRexActors() const;

    // Timers
    FTimerHandle TRexDetectionTimer;
    
    // State tracking
    float LastTRexDistance = 0.0f;
    bool bIsShaking = false;
    FVector LastTRexLocation = FVector::ZeroVector;
};