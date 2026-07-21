#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/StaticMeshActor.h"
#include "Math/UnrealMathUtility.h"
#include "../SharedTypes.h"
#include "EnvArt_CretaceousForestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_CretaceousPlantData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant Data")
    FString PlantName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant Data")
    float DensityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant Data")
    FVector2D HeightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant Data")
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant Data")
    int32 MaxInstancesPerCluster;

    FEnvArt_CretaceousPlantData()
    {
        PlantName = TEXT("DefaultPlant");
        DensityWeight = 1.0f;
        HeightRange = FVector2D(100.0f, 500.0f);
        ClusterRadius = 1000.0f;
        MaxInstancesPerCluster = 25;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_ForestAtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FRotator SunRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff;

    FEnvArt_ForestAtmosphereSettings()
    {
        SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
        SunIntensity = 3.5f;
        SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
        FogHeightFalloff = 0.2f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_ForestClusterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    FVector ClusterCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    TArray<AActor*> PlantActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    int32 PlantCount;

    FEnvArt_ForestClusterData()
    {
        ClusterCenter = FVector::ZeroVector;
        ClusterRadius = 1000.0f;
        PlantCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousForestManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousForestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core forest management
    UFUNCTION(BlueprintCallable, Category = "Forest Management")
    void InitializeForestBiome();

    UFUNCTION(BlueprintCallable, Category = "Forest Management")
    void GenerateVegetationClusters();

    UFUNCTION(BlueprintCallable, Category = "Forest Management")
    void SetupAtmosphericLighting();

    UFUNCTION(BlueprintCallable, Category = "Forest Management")
    void CreateVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Forest Management")
    void OptimizeForestPerformance();

    // Vegetation management
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnPlantCluster(const FEnvArt_CretaceousPlantData& PlantData, const FVector& ClusterCenter);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void RemoveExcessVegetation();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    int32 GetCurrentVegetationCount() const;

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void ValidateActorLimits();

    // Lighting management
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigureGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void CreateAmbientLightPoints();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateTimeOfDayLighting(float TimeOfDay);

    // Atmosphere management
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateForestAtmosphere(const FEnvArt_ForestAtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetForestFogParameters(float Density, const FLinearColor& Color, float HeightFalloff);

protected:
    // Forest configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Config")
    FVector ForestCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Config")
    float ForestRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Config")
    int32 MaxVegetationCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Config")
    int32 MaxClusterCount;

    // Cretaceous plant types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant Types")
    TArray<FEnvArt_CretaceousPlantData> CretaceousPlantTypes;

    // Forest clusters
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forest State")
    TArray<FEnvArt_ForestClusterData> VegetationClusters;

    // Atmosphere settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_ForestAtmosphereSettings AtmosphereSettings;

    // Lighting references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    AExponentialHeightFog* ForestFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    TArray<APointLight*> AmbientLights;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 CurrentActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorLimit;

private:
    // Internal helper methods
    void InitializeCretaceousPlantTypes();
    FVector GetRandomClusterPosition() const;
    bool IsValidPlantPosition(const FVector& Position) const;
    void CleanupOldVegetation();
    void UpdatePerformanceMetrics();
};