#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "World_ProceduralTerrainManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float WaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<class AActor*> SpawnedActors;

    FWorld_TerrainChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        BiomeType = EBiomeType::Savana;
        HeightVariation = 100.0f;
        WaterLevel = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    EWaterType WaterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float FlowRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    class AStaticMeshActor* WaterActor;

    FWorld_WaterFeature()
    {
        WaterType = EWaterType::Lake;
        Location = FVector::ZeroVector;
        Scale = FVector(10.0f, 10.0f, 0.1f);
        FlowRate = 0.0f;
        WaterActor = nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_ProceduralTerrainManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_ProceduralTerrainManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Terrain Generation
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainChunk(const FVector& ChunkCenter, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateFullTerrain(int32 ChunkCountX = 5, int32 ChunkCountY = 5);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    float GetHeightAtLocation(const FVector& WorldLocation) const;

    // Water System
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateWaterFeature(const FVector& Location, EWaterType WaterType, const FVector& Scale);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void GenerateRiverSystem(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateBiomeWaterFeatures(EBiomeType BiomeType, const FVector& BiomeCenter);

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetupBiomeTransitions();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetBiomeCenter(EBiomeType BiomeType) const;

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainLOD(const FVector& PlayerLocation, float ViewDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantChunks(const FVector& PlayerLocation, float CullDistance = 50000.0f);

    // Debug and Validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateTerrainIntegrity();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugDrawBiomeBoundaries();

protected:
    // Terrain Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_TerrainChunk> TerrainChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (AllowPrivateAccess = "true"))
    float ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (AllowPrivateAccess = "true"))
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (AllowPrivateAccess = "true"))
    int32 NoiseSeed;

    // Water System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_WaterFeature> WaterFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", meta = (AllowPrivateAccess = "true"))
    class UStaticMesh* WaterPlaneMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", meta = (AllowPrivateAccess = "true"))
    class UMaterialInterface* WaterMaterial;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, FVector> BiomeCenters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes", meta = (AllowPrivateAccess = "true"))
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes", meta = (AllowPrivateAccess = "true"))
    float BiomeTransitionWidth;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bEnableLODOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float LODUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float LastLODUpdateTime;

private:
    // Internal Helpers
    float GenerateNoiseValue(const FVector& Location) const;
    void ApplyBiomeModifiers(FWorld_TerrainChunk& Chunk) const;
    void SpawnBiomeVegetation(const FWorld_TerrainChunk& Chunk);
    void CleanupChunk(FWorld_TerrainChunk& Chunk);
};

#include "World_ProceduralTerrainManager.generated.h"