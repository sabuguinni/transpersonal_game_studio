#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_ScreenShakeController.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light       UMETA(DisplayName = "Light Shake"),
    Medium      UMETA(DisplayName = "Medium Shake"),
    Heavy       UMETA(DisplayName = "Heavy Shake"),
    Extreme     UMETA(DisplayName = "Extreme Shake")
};

USTRUCT(BlueprintType)
struct FAudio_ShakeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Parameters")
    float FalloffDistance = 2000.0f;

    FAudio_ShakeParameters()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        FalloffDistance = 2000.0f;
    }
};

/**
 * Screen shake controller for immersive audio-visual feedback
 * Triggers camera shake based on proximity to large dinosaurs and environmental events
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenShakeController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Trigger screen shake with specified intensity
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerShake(EAudio_ShakeIntensity Intensity, FVector SourceLocation);

    // Trigger custom shake with specific parameters
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeParameters& ShakeParams, FVector SourceLocation);

    // Check for nearby dinosaurs and trigger appropriate shake
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void CheckProximityShake();

    // Set shake enabled/disabled
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void SetShakeEnabled(bool bEnabled);

protected:
    // Shake parameters for different intensities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    FAudio_ShakeParameters LightShakeParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    FAudio_ShakeParameters MediumShakeParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    FAudio_ShakeParameters HeavyShakeParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    FAudio_ShakeParameters ExtremeShakeParams;

    // Detection settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float TRexShakeDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float LargeDinosaurShakeDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float CheckInterval = 0.5f;

    // State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bShakeEnabled = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float LastShakeTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float ShakeCooldown = 0.3f;

private:
    float LastCheckTime = 0.0f;
    APlayerController* CachedPlayerController = nullptr;

    // Internal methods
    void InitializeShakeParameters();
    FAudio_ShakeParameters GetShakeParamsForIntensity(EAudio_ShakeIntensity Intensity) const;
    float CalculateShakeIntensityByDistance(float Distance, float MaxDistance) const;
    bool CanTriggerShake() const;
    void ExecuteShake(const FAudio_ShakeParameters& Params, float IntensityMultiplier);
};