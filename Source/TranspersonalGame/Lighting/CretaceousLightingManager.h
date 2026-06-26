#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "CretaceousLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn       UMETA(DisplayName = "Dawn"),
    Morning    UMETA(DisplayName = "Morning"),
    Midday     UMETA(DisplayName = "Midday"),
    GoldenHour UMETA(DisplayName = "Golden Hour"),
    Dusk       UMETA(DisplayName = "Dusk"),
    Night      UMETA(DisplayName = "Night"),
};

USTRUCT(BlueprintType)
struct FLight_SunConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float Pitch = -38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float Intensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor LightColor = FLinearColor(1.0f, 0.878f, 0.619f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bAtmosphereSunLight = true;
};

USTRUCT(BlueprintType)
struct FLight_FogConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float HeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.45f, 0.62f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricFogExtinctionScale = 1.2f;
};

/**
 * ACretaceousLightingManager
 * Manages the full Cretaceous era lighting stack:
 * - Dynamic day/night cycle
 * - Warm golden hour sun with atmosphere
 * - Volumetric fog (humid jungle haze)
 * - Lumen GI + reflections
 * - SkyLight real-time capture
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Time of Day ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float TimeOfDaySeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float DayDurationSeconds = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    bool bEnableDynamicDayNight = true;

    // ── Sun Configuration ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLight_SunConfig SunConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    TObjectPtr<ADirectionalLight> SunActor;

    // ── Fog Configuration ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLight_FogConfig FogConfig;

    // ── Sky Light ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sky")
    TObjectPtr<ASkyLight> SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sky")
    float SkyLightIntensity = 2.5f;

    // ── Lumen Settings ───────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bEnableLumenGI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bEnableLumenReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bEnableVolumetricFog = true;

    // ── Blueprint-callable functions ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplySunConfig(const FLight_SunConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyFogConfig(const FLight_FogConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentSunPitch() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLinearColor GetCurrentSkyColor() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyGoldenHourPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyMidnightPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyDawnPreset();

private:
    void AdvanceDayNightCycle(float DeltaTime);
    float ComputeSunPitchFromTime(float NormalizedTime) const;
    FLinearColor ComputeSunColorFromTime(float NormalizedTime) const;
    void UpdateSunTransform(float Pitch);
    void UpdateSkyLightCapture();
};
