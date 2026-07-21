#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"
#include "Components/SphereComponent.h"
#include "VFX_ScreenShakeController.generated.h"

UENUM(BlueprintType)
enum class EVFX_ShakeIntensity : uint8
{
    Light       UMETA(DisplayName = "Light Shake"),
    Medium      UMETA(DisplayName = "Medium Shake"), 
    Heavy       UMETA(DisplayName = "Heavy Shake"),
    Extreme     UMETA(DisplayName = "Extreme Shake")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Config")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Config")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Config")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Config")
    float FadeInTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Config")
    float FadeOutTime = 0.5f;

    FVFX_ShakeConfig()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        FadeInTime = 0.1f;
        FadeOutTime = 0.5f;
    }
};

/**
 * VFX Screen Shake Controller
 * Manages camera shake effects triggered by dinosaur proximity, impacts, and environmental events
 * Integrates with Audio system for synchronized audio-visual feedback
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ScreenShakeController : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ScreenShakeController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Shake Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    TMap<EVFX_ShakeIntensity, FVFX_ShakeConfig> ShakeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ProximityCheckRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float TRexShakeDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float DinosaurShakeDistance = 300.0f;

    // Audio Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Integration")
    bool bSyncWithAudioSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Integration")
    float AudioSyncThreshold = 0.5f;

    // Performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ShakeCooldown = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxConcurrentShakes = 3;

private:
    // Internal state
    float LastShakeTime = 0.0f;
    int32 ActiveShakeCount = 0;
    TArray<AActor*> NearbyDinosaurs;
    APlayerController* CachedPlayerController = nullptr;

    // Proximity detection
    UFUNCTION()
    void CheckDinosaurProximity();

    UFUNCTION()
    void UpdateNearbyDinosaurs();

    // Shake execution
    UFUNCTION()
    void ExecuteShake(EVFX_ShakeIntensity Intensity, const FVector& SourceLocation);

    UFUNCTION()
    float CalculateShakeIntensity(float Distance, float MaxDistance) const;

    UFUNCTION()
    bool CanExecuteShake() const;

public:
    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerTRexApproachShake(const FVector& TRexLocation);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDinosaurFootstepShake(const FVector& FootstepLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerImpactShake(const FVector& ImpactLocation, float ImpactForce = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerEnvironmentalShake(EVFX_ShakeIntensity Intensity, float Duration = 1.0f);

    // Audio system integration
    UFUNCTION(BlueprintCallable, Category = "Audio Integration")
    void OnAudioEffectTriggered(const FVector& AudioLocation, float AudioIntensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Integration")
    void SetAudioSyncEnabled(bool bEnabled);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetShakeConfig(EVFX_ShakeIntensity Intensity, const FVFX_ShakeConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    FVFX_ShakeConfig GetShakeConfig(EVFX_ShakeIntensity Intensity) const;

    // Debug
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void TestScreenShake();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawProximityRadius() const;
};