#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Materials/MaterialParameterCollection.h"
#include "SharedTypes.h"
#include "EnvArt_AtmosphericRenderer.generated.h"

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    // Sun position and color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunElevation = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAzimuth = 135.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.86f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 8.0f;

    // Fog settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    // Atmospheric scattering
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereThickness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.004f;

    FEnvArt_AtmosphericSettings()
    {
        // Default Cretaceous period settings
    }
};

USTRUCT(BlueprintType)
struct FEnvArt_VolumetricFogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    FLinearColor VolumetricFogAlbedo = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricFogEmissive = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricFogExtinctionScale = 1.0f;

    FEnvArt_VolumetricFogSettings()
    {
        // Default volumetric fog for prehistoric atmosphere
    }
};

/**
 * Advanced atmospheric rendering system for Cretaceous period environments
 * Handles dynamic lighting, volumetric fog, and atmospheric scattering
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericRenderer : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericRenderer();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Atmospheric settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Settings")
    FEnvArt_VolumetricFogSettings VolumetricFogSettings;

    // Light references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ASkyLight* SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    AExponentialHeightFog* HeightFog;

    // Material parameter collection for atmospheric effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialParameterCollection* AtmosphericMPC;

    // Time of day control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDay = 15.0f; // 3 PM golden hour

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoAdvanceTime = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeAdvanceSpeed = 0.1f;

public:
    // Atmospheric control functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetSunPosition(float Elevation, float Azimuth);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetSunColor(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void SetFogDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyCretaceousPreset();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyGoldenHourPreset();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyMistyMorningPreset();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void UpdateAtmosphericParameters();

    // Volumetric fog control
    UFUNCTION(BlueprintCallable, Category = "Volumetric Fog")
    void SetVolumetricFogEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Fog")
    void SetVolumetricFogScattering(float ScatteringDistribution);

    // Editor utilities
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void RefreshLightingReferences();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void CreateAtmosphericLights();

protected:
    // Internal update functions
    void UpdateSunPosition();
    void UpdateFogSettings();
    void UpdateMaterialParameters();
    void FindLightingActors();

    // Time calculation helpers
    float CalculateSunElevation(float TimeHours);
    float CalculateSunAzimuth(float TimeHours);
    FLinearColor CalculateSunColor(float TimeHours);
};