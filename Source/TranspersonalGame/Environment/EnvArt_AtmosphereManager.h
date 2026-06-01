#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "../SharedTypes.h"
#include "EnvArt_AtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeAtmosphere
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogScatteringDistribution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor VolumetricFogAlbedo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogEmissive;

    FEnvArt_BiomeAtmosphere()
    {
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        VolumetricFogScatteringDistribution = 0.2f;
        VolumetricFogAlbedo = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
        VolumetricFogEmissive = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SunAngleElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SunAngleAzimuth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLinearColor SkyColor;

    FEnvArt_TimeOfDaySettings()
    {
        SunAngleElevation = -15.0f;
        SunAngleAzimuth = 45.0f;
        SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
        SunIntensity = 3.0f;
        SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Atmosphere settings per biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    TMap<EBiomeType, FEnvArt_BiomeAtmosphere> BiomeAtmosphereSettings;

    // Time of day presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    TMap<FString, FEnvArt_TimeOfDaySettings> TimeOfDayPresets;

    // Current atmosphere state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    EBiomeType CurrentBiome;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float CurrentTimeOfDay;

    // Fog actors per biome
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog System")
    TMap<EBiomeType, AExponentialHeightFog*> BiomeFogActors;

    // Main directional light
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    ADirectionalLight* MainSun;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetBiomeAtmosphere(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float TimeHour);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyTimeOfDayPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateBiomeFogActors();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphereForLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    EBiomeType GetBiomeAtLocation(const FVector& Location);

private:
    void InitializeDefaultSettings();
    void UpdateFogSettings(AExponentialHeightFog* FogActor, const FEnvArt_BiomeAtmosphere& Settings);
    void UpdateSunSettings(const FEnvArt_TimeOfDaySettings& Settings);
    FEnvArt_BiomeAtmosphere InterpolateBiomeAtmosphere(const FEnvArt_BiomeAtmosphere& A, const FEnvArt_BiomeAtmosphere& B, float Alpha);
};