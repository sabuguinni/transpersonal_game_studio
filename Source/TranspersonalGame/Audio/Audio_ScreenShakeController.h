#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_ScreenShakeController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    float FalloffRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    bool bUseDistanceAttenuation = true;

    FAudio_ShakeParameters()
    {
        Intensity = 1.0f;
        Duration = 0.5f;
        Frequency = 10.0f;
        FalloffRadius = 1000.0f;
        bUseDistanceAttenuation = true;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenShakeController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Screen shake functions
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerDinosaurFootstepShake(const FVector& FootstepLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerTRexApproachShake(const FVector& TRexLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeParameters& ShakeParams, const FVector& SourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void StopAllShakes();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Screen Shake")
    FAudio_ShakeParameters DefaultFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Screen Shake")
    FAudio_ShakeParameters TRexApproachShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Screen Shake")
    float MaxShakeDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Screen Shake")
    bool bEnableScreenShake = true;

private:
    // Internal shake management
    void ApplyScreenShake(const FAudio_ShakeParameters& ShakeParams, const FVector& SourceLocation);
    float CalculateShakeIntensity(const FVector& SourceLocation, float BaseIntensity, float FalloffRadius);

    UPROPERTY()
    class APlayerController* CachedPlayerController;

    // Shake tracking
    TArray<float> ActiveShakeDurations;
    float ShakeCooldownTimer = 0.0f;
    static constexpr float SHAKE_COOLDOWN = 0.1f;
};

#include "Audio_ScreenShakeController.generated.h"