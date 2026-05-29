#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "../../Core/SharedTypes.h"
#include "EnvArt_BiomeAtmosphere.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeAtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnableDustParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnablePollenParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bEnableAmbientSounds;

    FEnvArt_BiomeAtmosphereSettings()
    {
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f);
        SunIntensity = 3.0f;
        SunRotation = FRotator(-45.0f, 45.0f, 0.0f);
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f);
        SkyLightIntensity = 1.0f;
        bEnableDustParticles = true;
        bEnablePollenParticles = false;
        bEnableAmbientSounds = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_BiomeAtmosphere : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_BiomeAtmosphere();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* DustParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* PollenParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_BiomeAtmosphereSettings SwampSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_BiomeAtmosphereSettings ForestSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_BiomeAtmosphereSettings SavannaSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_BiomeAtmosphereSettings DesertSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_BiomeAtmosphereSettings MountainSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDuration;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyBiomeAtmosphere(EBiomeType InBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableVolumetricFog(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SpawnAtmosphericParticles();

private:
    void InitializeBiomeSettings();
    void UpdateLightingForTimeOfDay();
    FEnvArt_BiomeAtmosphereSettings GetCurrentBiomeSettings() const;
    void ApplyAtmosphereSettings(const FEnvArt_BiomeAtmosphereSettings& Settings);

    UPROPERTY()
    ADirectionalLight* CachedDirectionalLight;

    UPROPERTY()
    ASkyLight* CachedSkyLight;

    UPROPERTY()
    AExponentialHeightFog* CachedHeightFog;
};