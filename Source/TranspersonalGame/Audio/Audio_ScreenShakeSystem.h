#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Audio_ScreenShakeSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light       UMETA(DisplayName = "Light Shake"),
    Medium      UMETA(DisplayName = "Medium Shake"),
    Heavy       UMETA(DisplayName = "Heavy Shake"),
    Massive     UMETA(DisplayName = "Massive Shake")
};

USTRUCT(BlueprintType)
struct FAudio_ShakeParameters
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

    FAudio_ShakeParameters()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        bFadeIn = true;
        bFadeOut = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ScreenShakeSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ScreenShakeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Screen shake functions
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerShake(EAudio_ShakeIntensity Intensity, float Distance = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeParameters& ShakeParams);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerTRexFootstepShake(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDamageShake();

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void StopAllShakes();

protected:
    // Shake intensity presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    FAudio_ShakeParameters LightShakePreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    FAudio_ShakeParameters MediumShakePreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    FAudio_ShakeParameters HeavyShakePreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    FAudio_ShakeParameters MassiveShakePreset;

    // Distance-based shake settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Settings")
    float MaxShakeDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Settings")
    float MinShakeDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Settings")
    UCurveFloat* DistanceFalloffCurve;

private:
    // Internal shake management
    void InitializeShakePresets();
    float CalculateDistanceMultiplier(float Distance) const;
    void ApplyShakeToPlayer(const FAudio_ShakeParameters& ShakeParams);

    UPROPERTY()
    APlayerController* CachedPlayerController;
};