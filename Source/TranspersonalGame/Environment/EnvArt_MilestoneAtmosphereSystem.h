#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/AtmosphericFog.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/TriggerVolume.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "EnvArt_MilestoneAtmosphereSystem.generated.h"

/**
 * Milestone 1 Atmosphere System - Optimized for "WALK AROUND" gameplay
 * Creates immersive prehistoric atmosphere with golden hour lighting,
 * volumetric fog, particle effects, and ambient audio zones
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunElevation = -15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAzimuth = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 1.2f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AudioZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector Size = FVector(5000.0f, 5000.0f, 2000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString ZoneType = TEXT("Forest");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<class USoundBase> AmbientSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 0.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_MilestoneAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_MilestoneAtmosphereSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CORE ATMOSPHERE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ConfigureGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetupVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EstablishAudioZones();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnhanceSkyAtmosphere();

    // === LIGHTING CONTROL ===
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunAngle(float Elevation, float Azimuth);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunColor(FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunIntensity(float Intensity);

    // === FOG MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Fog")
    void CreateForestFog(const FVector& Location, float Density = 0.02f);

    UFUNCTION(BlueprintCallable, Category = "Fog")
    void UpdateFogSettings(const FEnvArt_AtmosphereSettings& Settings);

    // === AUDIO ZONES ===
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CreateAudioZone(const FEnvArt_AudioZone& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAudioZoneVolume(const FString& ZoneType, float NewVolume);

    // === MILESTONE 1 SPECIFIC ===
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Milestone")
    void SetupMilestone1Atmosphere();

    UFUNCTION(BlueprintCallable, Category = "Milestone")
    bool ValidateAtmosphereSetup() const;

protected:
    // === ATMOSPHERE SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FEnvArt_AtmosphereSettings AtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TArray<FEnvArt_AudioZone> AudioZones;

    // === ACTOR REFERENCES ===
    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<ADirectionalLight> SunLight = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<AAtmosphericFog> AtmosphericFog = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<ASkyAtmosphere> SkyAtmosphere = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TArray<TObjectPtr<AExponentialHeightFog>> ForestFogActors;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TArray<TObjectPtr<ATriggerVolume>> AudioTriggerVolumes;

    // === PARTICLE SYSTEMS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    TArray<FVector> ParticleSpawnPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    TObjectPtr<class UNiagaraSystem> DustParticleSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    TObjectPtr<class UNiagaraSystem> PollenParticleSystem = nullptr;

    // === PERFORMANCE TRACKING ===
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveFogActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveParticleSystems = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveAudioZones = 0;

private:
    // === INTERNAL HELPERS ===
    void FindOrCreateSunLight();
    void FindOrCreateAtmosphericFog();
    void FindOrCreateSkyAtmosphere();
    void CleanupOldFogActors();
    void ValidatePerformanceLimits();
};