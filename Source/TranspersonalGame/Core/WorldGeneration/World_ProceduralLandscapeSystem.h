#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Landscape/Landscape.h"
#include "LandscapeProxy.h"
#include "SharedTypes.h"
#include "World_ProceduralLandscapeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LandscapeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentCountX = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentCountY = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 QuadsPerComponent = 63;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 SectionsPerComponent = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    float HeightmapScale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector LandscapeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FRotator LandscapeRotation = FRotator::ZeroRotator;

    FWorld_LandscapeConfig()
    {
        ComponentCountX = 8;
        ComponentCountY = 8;
        QuadsPerComponent = 63;
        SectionsPerComponent = 1;
        HeightmapScale = 100.0f;
        LandscapeLocation = FVector::ZeroVector;
        LandscapeRotation = FRotator::ZeroRotator;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeLandscapeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 25000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainRoughness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ElevationVariance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<float> HeightmapData;

    FWorld_BiomeLandscapeData()
    {
        BiomeType = EBiomeType::Savana;
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 25000.0f;
        TerrainRoughness = 0.5f;
        ElevationVariance = 500.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_ProceduralLandscapeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_ProceduralLandscapeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core landscape generation
    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    bool GenerateProceduralLandscape();

    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    bool CreateBiomeLandscape(EBiomeType BiomeType, const FVector& BiomeCenter);

    UFUNCTION(BlueprintCallable, Category = "Landscape Generation")
    bool GenerateHeightmapForBiome(EBiomeType BiomeType, const FVector& BiomeCenter, TArray<float>& OutHeightmap);

    // Landscape configuration
    UFUNCTION(BlueprintCallable, Category = "Landscape Config")
    void SetLandscapeConfig(const FWorld_LandscapeConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Landscape Config", BlueprintPure)
    FWorld_LandscapeConfig GetLandscapeConfig() const { return LandscapeConfig; }

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void AddBiomeLandscapeData(const FWorld_BiomeLandscapeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    TArray<FWorld_BiomeLandscapeData> GetAllBiomeLandscapeData() const { return BiomeLandscapeDataArray; }

    // Terrain modification
    UFUNCTION(BlueprintCallable, Category = "Terrain Modification")
    bool ModifyTerrainHeight(const FVector& Location, float Radius, float HeightDelta);

    UFUNCTION(BlueprintCallable, Category = "Terrain Modification")
    bool SmoothTerrain(const FVector& Location, float Radius, float SmoothingStrength);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLandscapePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLandscapeLODDistances(float LOD0Distance, float LOD1Distance, float LOD2Distance);

    // Landscape queries
    UFUNCTION(BlueprintCallable, Category = "Landscape Queries", BlueprintPure)
    float GetHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Landscape Queries", BlueprintPure)
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Landscape Queries")
    bool GetLandscapeNormalAtLocation(const FVector& WorldLocation, FVector& OutNormal) const;

    // Editor utilities
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void GenerateAllBiomeLandscapes();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ClearAllLandscapes();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ValidateLandscapeSystem();

protected:
    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FWorld_LandscapeConfig LandscapeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FWorld_BiomeLandscapeData> BiomeLandscapeDataArray;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxLandscapeDrawDistance = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousLandscapes = 5;

    // Runtime data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<ALandscape*> GeneratedLandscapes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    bool bLandscapeSystemInitialized = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    float LastGenerationTime = 0.0f;

private:
    // Internal landscape generation
    bool CreateLandscapeActor(const FWorld_LandscapeConfig& Config, const FVector& Location, ALandscape*& OutLandscape);
    void ApplyHeightmapToLandscape(ALandscape* Landscape, const TArray<float>& HeightmapData);
    void ConfigureLandscapeMaterials(ALandscape* Landscape, EBiomeType BiomeType);
    
    // Heightmap generation algorithms
    float GeneratePerlinNoise(float X, float Y, float Scale, int32 Octaves) const;
    float GenerateBiomeSpecificHeight(EBiomeType BiomeType, float X, float Y, float BaseHeight) const;
    void ApplyBiomeModifiers(EBiomeType BiomeType, TArray<float>& HeightmapData, const FVector& BiomeCenter) const;

    // Performance monitoring
    void UpdatePerformanceMetrics();
    bool ShouldOptimizeLandscape(ALandscape* Landscape) const;
    void ApplyLODOptimization(ALandscape* Landscape) const;
};