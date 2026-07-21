#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "../SharedTypes.h"
#include "Audio_ScreenShakeComponent.generated.h"

class UCameraShakeBase;
class APlayerController;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float BaseIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float BaseDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float MaxRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    bool bUseDistanceFalloff = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float FalloffExponent = 2.0f;

    FAudio_ShakeSettings()
    {
        BaseIntensity = 1.0f;
        BaseDuration = 0.5f;
        MaxRange = 1000.0f;
        bUseDistanceFalloff = true;
        FalloffExponent = 2.0f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenShakeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeComponent();

protected:
    virtual void BeginPlay() override;

public:
    // Main shake trigger function
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerShake();

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerShakeAtLocation(const FVector& ShakeLocation);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeSettings& CustomSettings);

    // Preset shake types
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerFootstepShake(float CreatureSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerImpactShake(float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDamageShake(float DamageAmount);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    FAudio_ShakeSettings DefaultShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    FAudio_ShakeSettings FootstepShakePreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    FAudio_ShakeSettings ImpactShakePreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    FAudio_ShakeSettings DamageShakePreset;

    // Enable/disable shake
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    bool bShakeEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float GlobalShakeMultiplier = 1.0f;

protected:
    // Internal shake execution
    void ExecuteShake(const FVector& ShakeLocation, const FAudio_ShakeSettings& Settings);
    
    // Helper functions
    float CalculateDistanceFalloff(float Distance, const FAudio_ShakeSettings& Settings) const;
    APlayerController* GetPlayerController() const;
    
    // Shake state tracking
    UPROPERTY()
    float LastShakeTime = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings", meta = (AllowPrivateAccess = "true"))
    float MinShakeInterval = 0.1f; // Minimum time between shakes
};