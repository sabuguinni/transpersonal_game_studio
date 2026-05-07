#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Audio_ScreenShakeSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light       UMETA(DisplayName = "Light Shake"),
    Medium      UMETA(DisplayName = "Medium Shake"), 
    Heavy       UMETA(DisplayName = "Heavy Shake"),
    Extreme     UMETA(DisplayName = "Extreme Shake")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    bool bFadeIn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    bool bFadeOut = true;

    FAudio_ShakeParameters()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        bFadeIn = true;
        bFadeOut = true;
    }
};

/**
 * Screen shake system for audio feedback - creates camera shake effects
 * triggered by proximity to large dinosaurs, impacts, and environmental events
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_ScreenShakeSystem : public UObject
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeSystem();

    // Initialize the screen shake system
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void Initialize();

    // Trigger screen shake based on proximity to large creatures
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerProximityShake(AActor* SourceActor, float Distance, float CreatureSize);

    // Trigger damage impact shake
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerDamageShake(float DamageAmount, FVector ImpactLocation);

    // Trigger footstep shake for large creatures
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerFootstepShake(FVector FootstepLocation, float CreatureWeight);

    // Trigger environmental shake (earthquakes, volcanic activity)
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerEnvironmentalShake(EAudio_ShakeIntensity Intensity, FVector EpicenterLocation);

    // Custom shake with specific parameters
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeParameters& ShakeParams);

    // Stop all active shakes
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void StopAllShakes();

    // Set shake intensity multiplier (for accessibility/settings)
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void SetShakeIntensityMultiplier(float Multiplier);

protected:
    // Shake intensity multiplier for user settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ShakeIntensityMultiplier = 1.0f;

    // Maximum distance for proximity shakes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxProximityDistance = 5000.0f;

    // Minimum time between shakes to prevent spam
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ShakeCooldown = 0.1f;

    // Last shake time for cooldown
    UPROPERTY()
    float LastShakeTime = 0.0f;

    // Predefined shake parameters for different intensities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    TMap<EAudio_ShakeIntensity, FAudio_ShakeParameters> ShakePresets;

private:
    // Internal function to execute camera shake
    void ExecuteCameraShake(const FAudio_ShakeParameters& ShakeParams, FVector OptionalLocation = FVector::ZeroVector);

    // Calculate shake intensity based on distance
    float CalculateDistanceIntensity(float Distance, float MaxDistance) const;

    // Get player camera manager
    APlayerCameraManager* GetPlayerCameraManager() const;

    // Check if shake is on cooldown
    bool IsShakeOnCooldown() const;

    // Initialize default shake presets
    void InitializeShakePresets();
};