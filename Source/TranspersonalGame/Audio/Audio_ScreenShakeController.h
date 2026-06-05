#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Audio_ScreenShakeController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    float Falloff = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    bool bOrientShakeTowardsEpicenter = true;

    FAudio_ShakeSettings()
    {
        Intensity = 1.0f;
        Duration = 0.5f;
        Falloff = 1000.0f;
        bOrientShakeTowardsEpicenter = true;
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

    // Trigger screen shake based on distance and intensity
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerProximityShake(FVector SourceLocation, float MaxDistance, float BaseIntensity);

    // Trigger T-Rex footstep shake
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerTRexFootstep(FVector FootstepLocation);

    // Trigger damage impact shake
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerDamageShake(float DamageAmount);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    FAudio_ShakeSettings TRexShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    FAudio_ShakeSettings DamageShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    FAudio_ShakeSettings GenericShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* TRexFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* DamageImpactSound;

private:
    // Calculate shake intensity based on distance
    float CalculateShakeIntensity(FVector SourceLocation, FVector PlayerLocation, float MaxDistance, float BaseIntensity);

    // Apply camera shake to player
    void ApplyCameraShake(const FAudio_ShakeSettings& ShakeSettings, FVector EpicenterLocation);
};