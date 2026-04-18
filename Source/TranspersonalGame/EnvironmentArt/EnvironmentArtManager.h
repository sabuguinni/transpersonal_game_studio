#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Sound/AmbientSound.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "EnvironmentArtManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_BiomeType : uint8
{
    Forest          UMETA(DisplayName = "Dense Forest"),
    Grassland       UMETA(DisplayName = "Open Grassland"),
    Volcanic        UMETA(DisplayName = "Volcanic Region"),
    Riverside       UMETA(DisplayName = "River Valley"),
    Cave            UMETA(DisplayName = "Cave System"),
    Mountain        UMETA(DisplayName = "Mountain Ridge")
};

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn            UMETA(DisplayName = "Dawn"),
    Morning         UMETA(DisplayName = "Morning"),
    Noon            UMETA(DisplayName = "Noon"),
    Afternoon       UMETA(DisplayName = "Afternoon"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AtmosphereIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogStartDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_VegetationCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> MeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector Scale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 InstanceCount = 1;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvironmentArtManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvironmentArtManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Lighting Control
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void UpdateLighting(const FEnvArt_LightingSettings& LightingSettings);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SetGoldenHourLighting();

    // Atmosphere Control
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void UpdateFogSettings(const FEnvArt_FogSettings& FogSettings);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void CreateVolumetricFog(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SpawnAtmosphericParticles(FVector Location, EEnvArt_BiomeType BiomeType);

    // Vegetation Management
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PopulateBiome(EEnvArt_BiomeType BiomeType, FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PlaceVegetationCluster(const FEnvArt_VegetationCluster& ClusterData);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ScatterRocks(FVector Center, float Radius, int32 Count);

    // Audio Atmosphere
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SetupAmbientAudio(EEnvArt_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void PlayEnvironmentalSound(FVector Location, const FString& SoundName);

    // Material Control
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ApplyBiomeMaterials(EEnvArt_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void UpdateTerrainMaterial(UMaterialInterface* NewMaterial);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment Art")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment Art")
    FEnvArt_LightingSettings CurrentLighting;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment Art")
    FEnvArt_FogSettings CurrentFog;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment Art")
    TArray<FEnvArt_VegetationCluster> VegetationClusters;

    // Component References
    UPROPERTY(BlueprintReadOnly, Category = "Environment Art")
    TWeakObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Environment Art")
    TWeakObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "Environment Art")
    TWeakObjectPtr<APostProcessVolume> PostProcessVolume;

    // Asset References
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    TMap<EEnvArt_BiomeType, TSoftObjectPtr<UStaticMesh>> TreeMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    TMap<EEnvArt_BiomeType, TSoftObjectPtr<UStaticMesh>> RockMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    TMap<EEnvArt_BiomeType, TSoftObjectPtr<UMaterialInterface>> TerrainMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    TMap<EEnvArt_BiomeType, TSoftObjectPtr<USoundBase>> AmbientSounds;

private:
    void FindSceneComponents();
    void InitializeAssetMaps();
    FEnvArt_LightingSettings GetTimeOfDayLighting(EEnvArt_TimeOfDay TimeOfDay);
};