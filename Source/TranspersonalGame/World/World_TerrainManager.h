#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_TerrainManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bHasWater;

    FWorld_TerrainChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        ChunkSize = FVector(10000.0f, 10000.0f, 1000.0f);
        BiomeType = EBiomeType::Savanna;
        HeightVariation = 500.0f;
        bHasWater = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_TerrainManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_TerrainManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    TArray<FWorld_TerrainChunk> TerrainChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 MaxChunksPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float ChunkSpacing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    UStaticMesh* BaseTerrain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    UStaticMesh* WaterPlane;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* SavannaMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* SwampMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ForestMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* DesertMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* MountainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WaterMaterial;

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainForBiome(EBiomeType BiomeType, FVector BiomeCenter);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void CreateWaterBodies(EBiomeType BiomeType, FVector BiomeCenter);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void ClearExcessActors();

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    UMaterialInterface* GetMaterialForBiome(EBiomeType BiomeType);

private:
    UPROPERTY()
    TArray<AActor*> SpawnedTerrainActors;

    void SpawnTerrainChunk(const FWorld_TerrainChunk& Chunk);
    void ApplyBiomeMaterial(UStaticMeshComponent* MeshComp, EBiomeType BiomeType);
};