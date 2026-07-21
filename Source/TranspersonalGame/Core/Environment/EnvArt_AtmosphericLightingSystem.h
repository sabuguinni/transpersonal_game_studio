#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "EnvArt_AtmosphericLightingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunElevationAngle = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunAzimuthAngle = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SkyIntensity = 1.0f;
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
    FLinearColor FogInscatteringColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor VolumetricFogAlbedo = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogExtinctionScale = 1.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeAtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    FEnvArt_TimeOfDaySettings TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    FEnvArt_FogSettings Fog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    float AtmosphericScatteringIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    FLinearColor AtmosphericScatteringColor = FLinearColor(0.2f, 0.4f, 1.0f, 1.0f);
};

/**
 * Environment Artist atmospheric lighting system for creating cinematic lighting and fog effects
 * Manages golden hour lighting, volumetric fog, and biome-specific atmospheric conditions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Lighting references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ADirectionalLight* MainSunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ASkyLight* MainSkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class AExponentialHeightFog* MainHeightFog;

    // Atmosphere settings per biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TMap<EBiomeType, FEnvArt_BiomeAtmosphereSettings> BiomeAtmosphereSettings;

    // Current atmosphere state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    EBiomeType CurrentBiome = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    float CurrentTimeOfDay = 0.6f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    bool bEnableGoldenHour = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    bool bEnableDynamicTimeOfDay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    float TimeOfDaySpeed = 0.1f;

public:
    // Atmospheric control functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetBiomeAtmosphere(EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeOfDay(float TimeNormalized);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetVolumetricFogSettings(const FEnvArt_FogSettings& FogSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateSkyLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void InitializeBiomeSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting", CallInEditor = true)
    void ApplyCurrentSettings();

    // Utility functions
    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    FVector CalculateSunDirection() const;

    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    FLinearColor GetCurrentSunColor() const;

    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    float GetCurrentSunIntensity() const;

private:
    void InitializeDefaultBiomeSettings();
    FEnvArt_TimeOfDaySettings CalculateTimeOfDaySettings() const;
    FEnvArt_FogSettings CalculateFogSettings() const;
};