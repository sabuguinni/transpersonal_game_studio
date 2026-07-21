#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/AtmosphericFogComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/AtmosphericFog.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Particles/Emitter.h"
#include "Sound/AmbientSound.h"
#include "Components/AudioComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousAtmosphericEnhancer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.86f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunMultiplier = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereFogMultiplier = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float DensityMultiplier = 0.6f;

    FEnvArt_AtmosphericSettings()
    {
        // Default constructor with initialization above
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_PropSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString PropName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    EEnvArt_PropType PropType = EEnvArt_PropType::Rock;

    FEnvArt_PropSpawnData()
    {
        // Default constructor with initialization above
    }
};

/**
 * Atmospheric enhancement system for Cretaceous period environments
 * Manages golden hour lighting, volumetric fog, and environmental props
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousAtmosphericEnhancer : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousAtmosphericEnhancer();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Spawning")
    TArray<FEnvArt_PropSpawnData> FallenLogSpawns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Spawning")
    TArray<FEnvArt_PropSpawnData> RockSpawns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Spawning")
    TArray<FVector> ParticleSystemLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Spawning")
    TArray<FVector> AmbientSoundLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Spawning")
    TArray<FVector> FogVolumeLocations;

    // References to spawned actors
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedProps;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<class AEmitter*> SpawnedParticleSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<class AAmbientSound*> SpawnedAmbientSounds;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<class AExponentialHeightFog*> SpawnedFogVolumes;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Enhancement")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Enhancement")
    void EnhanceAtmosphericFog();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Enhancement")
    void SpawnEnvironmentalProps();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Enhancement")
    void SpawnAtmosphericParticles();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Enhancement")
    void SpawnAmbientSounds();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Enhancement")
    void CreateVolumetricFogVolumes();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Enhancement")
    void ApplyAllAtmosphericEnhancements();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Enhancement")
    void ClearAllSpawnedActors();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Enhancement", CallInEditor = true)
    void PreviewAtmosphericSettings();

protected:
    UFUNCTION()
    AStaticMeshActor* SpawnPropActor(const FEnvArt_PropSpawnData& SpawnData);

    UFUNCTION()
    class ADirectionalLight* FindDirectionalLight();

    UFUNCTION()
    class AAtmosphericFog* FindAtmosphericFog();

    UFUNCTION()
    UStaticMesh* GetPlaceholderMeshForPropType(EEnvArt_PropType PropType);

private:
    void InitializeDefaultSpawnData();
    void SetupFallenLogSpawns();
    void SetupRockSpawns();
    void SetupParticleLocations();
    void SetupAmbientSoundLocations();
    void SetupFogVolumeLocations();
};