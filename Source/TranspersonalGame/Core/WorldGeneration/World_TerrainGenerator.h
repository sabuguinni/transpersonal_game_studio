#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "World_BiomeSystem.h"
#include "SharedTypes.h"
#include "World_TerrainGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkSize = FVector(1000.0f, 1000.0f, 500.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_BiomeType DominantBiome = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightVariation = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bHasWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsGenerated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<FVector> RockFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<FVector> WaterBodies;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_TerrainGenerator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_TerrainGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 ChunkGridSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float ChunkSize = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float MaxTerrainHeight = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float NoiseScale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 NoiseOctaves = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float NoisePersistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    bool bAutoGenerateOnStart = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    bool bGenerateWaterBodies = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    bool bGenerateRockFormations = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* GrassMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* RockMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* TerrainChunkMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* RockMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* WaterPlaneMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<FWorld_TerrainChunk> TerrainChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    UWorld_BiomeSystem* BiomeSystemRef;

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateFullTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainChunk(int32 ChunkX, int32 ChunkY);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ClearAllTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    float GetHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    FWorld_TerrainChunk GetChunkAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void AddWaterBody(const FVector& Location, float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void AddRockFormation(const FVector& Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    TArray<FVector> GetAllWaterLocations() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    TArray<FVector> GetAllRockLocations() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Generation")
    void RegenerateTerrainEditor();

protected:
    UFUNCTION()
    void InitializeBiomeSystem();

    UFUNCTION()
    float GeneratePerlinNoise(float X, float Y) const;

    UFUNCTION()
    void CreateTerrainMesh(const FWorld_TerrainChunk& Chunk);

    UFUNCTION()
    void SpawnWaterBodies(const FWorld_TerrainChunk& Chunk);

    UFUNCTION()
    void SpawnRockFormations(const FWorld_TerrainChunk& Chunk);

    UPROPERTY()
    TArray<UStaticMeshComponent*> SpawnedTerrainMeshes;

    UPROPERTY()
    TArray<UStaticMeshComponent*> SpawnedWaterMeshes;

    UPROPERTY()
    TArray<UStaticMeshComponent*> SpawnedRockMeshes;

    UPROPERTY()
    bool bIsInitialized = false;

    UPROPERTY()
    int32 RandomSeed = 12345;
};