#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Landscape/Classes/LandscapeStreamingProxy.h"
#include "Components/ActorComponent.h"
#include "World_MassiveTerrainManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Pantano     UMETA(DisplayName = "Swamp"),
    Floresta    UMETA(DisplayName = "Forest"),
    Savana      UMETA(DisplayName = "Savanna"),
    Deserto     UMETA(DisplayName = "Desert"),
    Montanha    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ChunkCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChunkSize = 5000000.0f; // 5km in UU

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ChunkX = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ChunkY = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<ALandscapeStreamingProxy> LandscapeProxy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLoaded = false;

    FWorld_BiomeChunk()
    {
        BiomeType = EWorld_BiomeType::Savana;
        ChunkCenter = FVector::ZeroVector;
        ChunkSize = 5000000.0f;
        ChunkX = 0;
        ChunkY = 0;
        bIsLoaded = false;
    }
};

USTRUCT(BlueprintType)
struct FWorld_VegetationSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinScale = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxScale = 2.0f;

    FWorld_VegetationSpawnData()
    {
        BiomeType = EWorld_BiomeType::Savana;
        VegetationTypes = {"Tree", "Rock", "Grass"};
        Density = 1.0f;
        MinScale = 0.5f;
        MaxScale = 2.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_MassiveTerrainManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_MassiveTerrainManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float TotalWorldSize = 20000000.0f; // 200km in UU

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float ChunkSize = 5000000.0f; // 5km per chunk

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 ChunksPerSide = 4; // 4x4 = 16 chunks

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeChunk> BiomeChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FWorld_VegetationSpawnData> VegetationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingDistance = 10000000.0f; // 10km streaming radius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVegetationPerChunk = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableWorldStreaming = true;

public:
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void InitializeMassiveTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void CreateLandscapeChunks();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EWorld_BiomeType GetBiomeTypeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnVegetationInBiome(EWorld_BiomeType BiomeType, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateChunkStreaming(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Water")
    void CreateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainLOD();

private:
    void SetupBiomeDefinitions();
    FVector GetBiomeCenterLocation(EWorld_BiomeType BiomeType) const;
    float GetBiomeRadius(EWorld_BiomeType BiomeType) const;
    void CleanupDuplicateActors();
};