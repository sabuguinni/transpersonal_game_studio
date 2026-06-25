#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BiomeVegetationSpawner.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Jungle     UMETA(DisplayName = "Jungle"),
    Savanna    UMETA(DisplayName = "Savanna"),
    Swamp      UMETA(DisplayName = "Swamp"),
    Rocky      UMETA(DisplayName = "Rocky"),
    Riverbank  UMETA(DisplayName = "Riverbank")
};

USTRUCT(BlueprintType)
struct FWorld_VegetationEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float MinScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float MaxScale = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float SpawnWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FWorld_VegetationEntry> VegetationEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TreeDensity = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float SpawnRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;
};

/**
 * ABiomeVegetationSpawner
 * Procedurally spawns biome-appropriate vegetation around a center point.
 * Agent #5 — Procedural World Generator
 */
UCLASS(ClassGroup = (WorldGen), meta = (DisplayName = "Biome Vegetation Spawner"))
class TRANSPERSONALGAME_API ABiomeVegetationSpawner : public AActor
{
    GENERATED_BODY()

public:
    ABiomeVegetationSpawner();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FWorld_BiomeConfig BiomeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 RandomSeed = 42;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bSpawnOnBeginPlay = true;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "WorldGen")
    void SpawnVegetation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "WorldGen")
    void ClearSpawnedVegetation();

    UFUNCTION(BlueprintPure, Category = "WorldGen")
    int32 GetSpawnedCount() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<AActor*> SpawnedActors;

    FVector GetRandomPositionInBiome(float Radius, FRandomStream& Stream) const;
    float GetGroundHeight(const FVector& Location) const;
};
