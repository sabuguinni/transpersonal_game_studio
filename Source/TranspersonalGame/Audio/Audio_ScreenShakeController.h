#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"
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
struct TRANSPERSONALGAME_API FAudio_ShakeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    bool bFadeIn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    bool bFadeOut = true;

    FAudio_ShakeSettings()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        bFadeIn = true;
        bFadeOut = true;
    }
};

/**
 * Screen shake controller for immersive audio-visual feedback
 * Handles camera shake effects triggered by audio events like T-Rex footsteps
 */
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
    void TriggerTRexFootstepShake(float Distance = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomShake(EAudio_ShakeIntensity Intensity, float Duration = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerShakeFromSettings(const FAudio_ShakeSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void StopAllShakes();

    // Distance-based shake calculation
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio Screen Shake")
    float CalculateShakeIntensityFromDistance(float Distance, float MaxDistance = 2000.0f) const;

protected:
    // Shake intensity presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    FAudio_ShakeSettings LightShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    FAudio_ShakeSettings MediumShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    FAudio_ShakeSettings HeavyShakeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    FAudio_ShakeSettings ExtremeShakeSettings;

    // T-Rex specific settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Shake")
    float TRexMaxShakeDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Shake")
    float TRexMinShakeDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Shake")
    float TRexShakeIntensityMultiplier = 2.0f;

    // Internal state
    UPROPERTY()
    APlayerController* CachedPlayerController;

    UPROPERTY()
    TSubclassOf<UCameraShakeBase> DefaultShakeClass;

private:
    void InitializeShakePresets();
    void CachePlayerController();
    UCameraShakeBase* CreateShakeInstance(const FAudio_ShakeSettings& Settings);
};