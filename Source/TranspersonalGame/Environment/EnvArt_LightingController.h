#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "SharedTypes.h"
#include "EnvArt_LightingController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float VolumetricScatteringIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bCastVolumetricShadow = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogAlbedo = 1.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieAnisotropy = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor RayleighScattering = FLinearColor(0.331f, 0.558f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor MieScattering = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_LightingController : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_LightingController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Lighting references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ADirectionalLight* DirectionalLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    AExponentialHeightFog* FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ASkyLight* SkyLightActor;

    // Lighting settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FEnvArt_TimeOfDaySettings TimeOfDaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FEnvArt_FogSettings FogSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    // Time of day control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Control", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDay = 16.0f; // 4 PM golden hour

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Control")
    bool bAutoUpdateTime = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Control")
    float TimeSpeed = 1.0f;

public:
    virtual void Tick(float DeltaTime) override;

    // Lighting control functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateDirectionalLight();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void FindLightingActors();

    // Preset functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Presets")
    void ApplyDawnPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting Presets")
    void ApplyNoonPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting Presets")
    void ApplyGoldenHourPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting Presets")
    void ApplyDuskPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting Presets")
    void ApplyNightPreset();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Utilities")
    FRotator CalculateSunRotation(float TimeHour) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting Utilities")
    FLinearColor GetSunColorForTime(float TimeHour) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting Utilities")
    float GetSunIntensityForTime(float TimeHour) const;

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void RefreshLightingReferences();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ApplyCurrentSettings();

private:
    void UpdateTimeOfDay(float DeltaTime);
    void InterpolateLightingSettings(float Alpha);
    FLinearColor InterpolateColor(const FLinearColor& ColorA, const FLinearColor& ColorB, float Alpha) const;
};