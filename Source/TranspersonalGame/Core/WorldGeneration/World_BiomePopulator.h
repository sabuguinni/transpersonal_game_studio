#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_BiomePopulator.generated.h"

/**
 * World Biome Populator - Manages procedural population of biomes with environmental actors
 * Handles the 5 biome system: Swamp, Forest, Savanna, Desert, Mountain
 * Uses exact coordinates from brain memory for accurate placement
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomePopulator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomePopulator();

protected:
    virtual void BeginPlay() override;

public:
    // Biome population methods
    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void PopulateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void PopulateSwampBiome();

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void PopulateForestBiome();

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void PopulateSavannaBiome();

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void PopulateDesertBiome();

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void PopulateMountainBiome();

    // Cleanup methods
    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void CleanupDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "World Generation", CallInEditor)
    void RemoveBiomeActors();

    // Utility methods
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FVector GetRandomLocationInBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    int32 GetActorCountInBiome(EWorld_BiomeType BiomeType);

protected:
    // Biome configuration data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Settings")
    TMap<EWorld_BiomeType, FWorld_BiomeData> BiomeConfigurations;

    // Actor spawning settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population Settings")
    int32 MaxActorsPerBiome = 50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population Settings")
    int32 ActorsPerChunk = 15;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Population Settings")
    float MinDistanceBetweenActors = 500.0f;

    // Performance monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    int32 TotalActorsSpawned = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    float LastPopulationTime = 0.0f;

private:
    // Internal helper methods
    void InitializeBiomeData();
    FVector GetBiomeBounds(EWorld_BiomeType BiomeType, bool bGetMin);
    bool IsLocationValid(const FVector& Location, EWorld_BiomeType BiomeType);
    void SpawnEnvironmentalActor(const FVector& Location, const FString& ActorLabel, EWorld_BiomeType BiomeType);

    // Biome-specific spawning helpers
    void SpawnSwampActors(int32 Count);
    void SpawnForestActors(int32 Count);
    void SpawnSavannaActors(int32 Count);
    void SpawnDesertActors(int32 Count);
    void SpawnMountainActors(int32 Count);
};