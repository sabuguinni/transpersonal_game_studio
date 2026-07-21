#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_ProceduralTerrainSpawner.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString TerrainType;

    FWorld_TerrainSpawnData()
    {
        Location = FVector::ZeroVector;
        Scale = FVector::OneVector;
        Rotation = FRotator::ZeroRotator;
        BiomeType = EWorld_BiomeType::Savana;
        TerrainType = TEXT("Rock");
    }
};

/**
 * Procedural Terrain Spawner - Creates terrain actors for biome population
 * Spawns rocks, hills, terrain variations across the 5 biomes
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_ProceduralTerrainSpawner : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_ProceduralTerrainSpawner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome coordinates from memory ID 709
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EWorld_BiomeType, FVector> BiomeLocations;

    // Terrain spawn settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Spawning")
    int32 TerrainActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Spawning")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Spawning")
    float MinTerrainScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Spawning")
    float MaxTerrainScale;

    // Spawned terrain tracking
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    TArray<FWorld_TerrainSpawnData> SpawnedTerrainActors;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 TotalTerrainActorsSpawned;

    // Main terrain spawning functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Spawning")
    void SpawnTerrainForAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Terrain Spawning")
    void SpawnTerrainForBiome(EWorld_BiomeType BiomeType, int32 NumActors);

    UFUNCTION(BlueprintCallable, Category = "Terrain Spawning")
    AStaticMeshActor* SpawnSingleTerrainActor(const FVector& Location, const FVector& Scale, EWorld_BiomeType BiomeType);

    // Biome population counting
    UFUNCTION(BlueprintCallable, Category = "Biome Analysis")
    int32 CountActorsInBiome(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Analysis")
    TMap<EWorld_BiomeType, int32> GetBiomePopulationCounts() const;

    // Terrain cleanup
    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void ClearTerrainActors();

    UFUNCTION(BlueprintCallable, Category = "Terrain Management")
    void ClearBiomeTerrainActors(EWorld_BiomeType BiomeType);

private:
    void InitializeBiomeLocations();
    FVector GenerateRandomLocationInBiome(EWorld_BiomeType BiomeType) const;
    FVector GenerateRandomScale() const;
    FString GetTerrainTypeForBiome(EWorld_BiomeType BiomeType) const;
};