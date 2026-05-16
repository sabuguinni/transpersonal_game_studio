#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "EnvironmentalAtmosphere.generated.h"

/**
 * Environmental Atmosphere Actor
 * Creates and manages atmospheric elements for biomes including lighting, fog, particles, and ambient audio
 * Designed for Cretaceous period prehistoric environments with realistic atmospheric effects
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentalAtmosphere : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentalAtmosphere();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Root scene component for atmospheric elements */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    USceneComponent* AtmosphereRoot;

    /** Particle system for dust and pollen effects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    UParticleSystemComponent* DustParticles;

    /** Audio component for ambient wind and nature sounds */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    UAudioComponent* AmbientAudio;

    /** Static mesh for wind effect visualization markers */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    UStaticMeshComponent* WindMarker;

public:
    /** Atmospheric intensity (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AtmosphericIntensity;

    /** Dust particle density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float DustDensity;

    /** Wind strength affecting particles and audio */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float WindStrength;

    /** Time of day factor for lighting adjustments */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDay;

    /** Biome type for atmospheric presets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings")
    FString BiomeType;

    /** Enable golden hour lighting effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bEnableGoldenHour;

    /** Enable volumetric fog effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bEnableVolumetricFog;

    /** Fog density for atmospheric depth */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FogDensity;

    /** Sun angle for directional lighting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "-90.0", ClampMax = "90.0"))
    float SunAngle;

    /** Ambient light color for atmospheric mood */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientLightColor;

public:
    /** Initialize atmospheric effects for specific biome */
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeBiomeAtmosphere(const FString& InBiomeType);

    /** Update atmospheric effects based on time and weather */
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphericEffects(float DeltaSeconds);

    /** Set golden hour lighting configuration */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetGoldenHourLighting(bool bEnable, float InSunAngle = 15.0f);

    /** Configure volumetric fog for atmospheric depth */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigureVolumetricFog(float Density, const FLinearColor& FogColor);

    /** Spawn dust particles at specified location */
    UFUNCTION(BlueprintCallable, Category = "Particles")
    void SpawnDustParticles(const FVector& Location, float Intensity = 1.0f);

    /** Update wind effects and particle movement */
    UFUNCTION(BlueprintCallable, Category = "Wind")
    void UpdateWindEffects(float Strength, const FVector& Direction);

    /** Get current atmospheric settings as string for debugging */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetAtmosphericDebugInfo() const;

protected:
    /** Internal method to setup particle systems */
    void SetupParticleSystems();

    /** Internal method to configure audio components */
    void SetupAmbientAudio();

    /** Internal method to apply biome-specific presets */
    void ApplyBiomePresets(const FString& BiomeName);

    /** Current wind direction vector */
    FVector CurrentWindDirection;

    /** Timer for atmospheric updates */
    float AtmosphereUpdateTimer;

    /** Reference to world directional light */
    UPROPERTY()
    ADirectionalLight* WorldSun;
};