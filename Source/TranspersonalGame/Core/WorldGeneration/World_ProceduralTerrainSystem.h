#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "SharedTypes.h"
#include "World_ProceduralTerrainSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> BiomeMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor;

    FWorld_BiomeDefinition()
    {
        BiomeName = TEXT("Default");
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        VegetationDensity = 0.5f;
        RockDensity = 0.3f;
        BiomeColor = FLinearColor::Green;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Roughness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TSoftObjectPtr<UMaterialInterface> LayerMaterial;

    FWorld_TerrainLayer()
    {
        LayerName = TEXT("BaseLayer");
        Height = 0.0f;
        Roughness = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_ProceduralTerrainSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_ProceduralTerrainSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core terrain generation
    UFUNCTION(BlueprintCallable, Category = "Procedural Terrain")
    void GenerateProceduralTerrain();

    UFUNCTION(BlueprintCallable, Category = "Procedural Terrain")
    void GenerateBiomes();

    UFUNCTION(BlueprintCallable, Category = "Procedural Terrain")
    void PopulateBiomeWithObjects(const FWorld_BiomeDefinition& Biome);

    UFUNCTION(BlueprintCallable, Category = "Procedural Terrain")
    void CreateTerrainLayers();

    // Landscape manipulation
    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void CreateLandscapeActor(int32 ComponentCountX, int32 ComponentCountY, int32 QuadsPerComponent);

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void SculptLandscapeHeight(FVector WorldLocation, float Radius, float Strength);

    UFUNCTION(BlueprintCallable, Category = "Landscape")
    void PaintLandscapeLayer(FVector WorldLocation, float Radius, const FString& LayerName, float Strength);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantObjects(FVector PlayerLocation, float CullDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveObjectCount() const;

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeDefinition GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void AddCustomBiome(const FWorld_BiomeDefinition& NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<FWorld_BiomeDefinition> GetAllBiomes() const;

    // Terrain queries
    UFUNCTION(BlueprintCallable, Category = "Terrain Query")
    float GetTerrainHeightAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Query")
    FVector GetTerrainNormalAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Query")
    bool IsLocationInWater(FVector WorldLocation) const;

protected:
    // Biome definitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeDefinition> BiomeDefinitions;

    // Terrain layers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<FWorld_TerrainLayer> TerrainLayers;

    // Landscape reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape")
    class ALandscape* LandscapeActor;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxObjectsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

    // Generation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 RandomSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float HeightMultiplier;

    // Runtime tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedObjects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    int32 TotalObjectsSpawned;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    bool bTerrainGenerated;

private:
    // Internal generation methods
    void InitializeDefaultBiomes();
    void SpawnObjectInBiome(const FWorld_BiomeDefinition& Biome, UStaticMesh* Mesh, FVector Location, FRotator Rotation);
    float GenerateNoiseValue(float X, float Y) const;
    FVector GetRandomLocationInBiome(const FWorld_BiomeDefinition& Biome) const;
    UStaticMesh* SelectRandomMeshForBiome(const FWorld_BiomeDefinition& Biome) const;
    void CleanupOldObjects();
};